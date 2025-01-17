/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict;"

const {classes: Cc, interfaces: Ci, utils: Cu, results: Cr} = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");


XPCOMUtils.defineLazyModuleGetter(this, 'LogManager',
  'resource://services-common/logmanager.js');

XPCOMUtils.defineLazyModuleGetter(this, 'Log',
  'resource://gre/modules/Log.jsm');

XPCOMUtils.defineLazyModuleGetter(this, 'Preferences',
  'resource://gre/modules/Preferences.jsm');

XPCOMUtils.defineLazyModuleGetter(this, 'setTimeout',
  'resource://gre/modules/Timer.jsm');
XPCOMUtils.defineLazyModuleGetter(this, 'clearTimeout',
  'resource://gre/modules/Timer.jsm');

Cu.import('resource://gre/modules/Task.jsm');

this.EXPORTED_SYMBOLS = ["ReadingListScheduler"];

// A list of "external" observer topics that may cause us to change when we
// sync.
const OBSERVERS = [
  // We don't sync when offline and restart when online.
  "network:offline-status-changed",
  // FxA notifications also cause us to check if we should sync.
  "fxaccounts:onverified",
  // When something notices a local change to an item.
  "readinglist:item-changed",
  // some notifications the engine might send if we have been requested to backoff.
  "readinglist:backoff-requested",
  // request to sync now
  "readinglist:user-sync",

];

///////// A temp object until we get our "engine"
let engine = {
  ERROR_AUTHENTICATION: "authentication error",
  sync: Task.async(function* () {
  }),
}

let prefs = new Preferences("readinglist.scheduler.");

// A helper to manage our interval values.
let intervals = {
  // Getters for our intervals.
  _fixupIntervalPref(prefName, def) {
    // All pref values are seconds, but we return ms.
    return prefs.get(prefName, def) * 1000;
  },

  // How long after startup do we do an initial sync?
  get initial() this._fixupIntervalPref("initial", 20), // 20 seconds.
  // Every interval after the first.
  get schedule() this._fixupIntervalPref("schedule", 2 * 60 * 60), // 2 hours
  // After we've been told an item has changed
  get dirty() this._fixupIntervalPref("dirty", 2 * 60), // 2 mins
  // After an error
  get retry() this._fixupIntervalPref("retry", 2 * 60), // 2 mins
};

// This is the implementation, but it's not exposed directly.
function InternalScheduler() {
  // oh, I don't know what logs yet - let's guess!
  let logs = [
    "browserwindow.syncui",
    "FirefoxAccounts",
    "readinglist.api",
    "readinglist.serverclient",
    "readinglist.sync",
  ];

  this._logManager = new LogManager("readinglist.", logs, "readinglist");
  this.log = Log.repository.getLogger("readinglist.scheduler");
  this.log.info("readinglist scheduler created.")
  this.state = this.STATE_OK;

  // don't this.init() here, but instead at the module level - tests want to
  // add hooks before it is called.
}

InternalScheduler.prototype = {
  // When the next scheduled sync should happen.  If we can sync, there will
  // be a timer set to fire then. If we can't sync there will not be a timer,
  // but it will be set to fire then as soon as we can.
  _nextScheduledSync: null,
  // The time when the most-recent "backoff request" expires - we will never
  // schedule a new timer before this.
  _backoffUntil: 0,
  // Our current timer.
  _timer: null,
  // Our timer fires a promise - _timerRunning is true until it resolves or
  // rejects.
  _timerRunning: false,
  // Our sync engine - XXX - maybe just a callback?
  _engine: engine,

  // Our state variable and constants.
  state: null,
  STATE_OK: "ok",
  STATE_ERROR_AUTHENTICATION: "authentication error",
  STATE_ERROR_OTHER: "other error",

  init() {
    this.log.info("scheduler initialzing");
    this._observe = this.observe.bind(this);
    for (let notification of OBSERVERS) {
      Services.obs.addObserver(this._observe, notification, false);
    }
    this._nextScheduledSync = Date.now() + intervals.initial;
    this._setupTimer();
  },

  // Note: only called by tests.
  finalize() {
    this.log.info("scheduler finalizing");
    this._clearTimer();
    for (let notification of OBSERVERS) {
      Services.obs.removeObserver(this._observe, notification);
    }
    this._observe = null;
  },

  observe(subject, topic, data) {
    this.log.debug("observed ${}", topic);
    switch (topic) {
      case "readinglist:backoff-requested": {
        // The subject comes in as a string, a number of seconds.
        let interval = parseInt(data, 10);
        if (isNaN(interval)) {
          this.log.warn("Backoff request had non-numeric value", data);
          return;
        }
        this.log.info("Received a request to backoff for ${} seconds", interval);
        this._backoffUntil = Date.now() + interval * 1000;
        this._maybeReschedule(0);
        break;
      }
      case "readinglist:local:dirty":
        this._maybeReschedule(intervals.dirty);
        break;
      case "readinglist:user-sync":
        this._syncNow();
        break;
      case "fxaccounts:onverified":
        // If we were in an authentication error state, reset that now.
        if (this.state == this.STATE_ERROR_AUTHENTICATION) {
          this.state = this.STATE_OK;
        }
        break;

      // The rest just indicate that now is probably a good time to check if
      // we can sync as normal using whatever schedule was previously set.
      default:
        break;
    }
    // When observers fire we ignore the current sync error state as the
    // notification may indicate it's been resolved.
    this._setupTimer(true);
  },

  // Is the current error state such that we shouldn't schedule a new sync.
  _isBlockedOnError() {
    // this needs more thought...
    return this.state == this.STATE_ERROR_AUTHENTICATION;
  },

  // canSync indicates if we can currently sync.
  _canSync(ignoreBlockingErrors = false) {
    if (Services.io.offline) {
      this.log.info("canSync=false - we are offline");
      return false;
    }
    if (!ignoreBlockingErrors && this._isBlockedOnError()) {
      this.log.info("canSync=false - we are in a blocked error state", this.state);
      return false;
    }
    this.log.info("canSync=true");
    return true;
  },

  // _setupTimer checks the current state and the environment to see when
  // we should next sync and creates the timer with the appropriate delay.
  _setupTimer(ignoreBlockingErrors = false) {
    if (!this._canSync(ignoreBlockingErrors)) {
      this._clearTimer();
      return;
    }
    if (this._timer) {
      let when = new Date(this._nextScheduledSync);
      let delay = this._nextScheduledSync - Date.now();
      this.log.info("checkStatus - already have a timer - will fire in ${delay}ms at ${when}",
                    {delay, when});
      return;
    }
    if (this._timerRunning) {
      this.log.info("checkStatus - currently syncing");
      return;
    }
    // no timer and we can sync, so start a new one.
    let now = Date.now();
    let delay = Math.max(0, this._nextScheduledSync - now);
    let when = new Date(now + delay);
    this.log.info("next scheduled sync is in ${delay}ms (at ${when})", {delay, when})
    this._timer = this._setTimeout(delay);
  },

  // Something (possibly naively) thinks the next sync should happen in
  // delay-ms. If there's a backoff in progress, ignore the requested delay
  // and use the back-off. If there's already a timer scheduled for earlier
  // than delay, let the earlier timer remain. Otherwise, use the requested
  // delay.
  _maybeReschedule(delay) {
    // If there's no delay specified and there's nothing currently scheduled,
    // it means a backoff request while the sync is actually running - there's
    // no need to do anything here - the next reschedule after the sync
    // completes will take the backoff into account.
    if (!delay && !this._nextScheduledSync) {
      this.log.debug("_maybeReschedule ignoring a backoff request while running");
      return;
    }
    let now = Date.now();
    if (!this._nextScheduledSync) {
      this._nextScheduledSync = now + delay;
    }
    // If there is something currently scheduled before the requested delay,
    // keep the existing value (eg, if we have a timer firing in 1 second, and
    // get a "dirty" notification that says we should sync in 2 seconds, we
    // keep the 1 second value)
    this._nextScheduledSync = Math.min(this._nextScheduledSync, now + delay);
    // But we still need to honor a backoff.
    this._nextScheduledSync = Math.max(this._nextScheduledSync, this._backoffUntil);
    // And always create a new timer next time _setupTimer is called.
    this._clearTimer();
  },

  // callback for when the timer fires.
  _doSync() {
    this.log.debug("starting sync");
    this._timer = null;
    this._timerRunning = true;
    // flag that there's no new schedule yet, so a request coming in while
    // we are running does the right thing.
    this._nextScheduledSync = 0;
    Services.obs.notifyObservers(null, "readinglist:sync:start", null);
    this._engine.sync().then(() => {
      this.log.info("Sync completed successfully");
      // Write a pref in the same format used to services/sync to indicate
      // the last success.
      prefs.set("lastSync", new Date().toString());
      this.state = this.STATE_OK;
      this._logManager.resetFileLog(this._logManager.REASON_SUCCESS);
      Services.obs.notifyObservers(null, "readinglist:sync:finish", null);
      return intervals.schedule;
    }).catch(err => {
      this.log.error("Sync failed", err);
      // XXX - how to detect an auth error?
      this.state = err == this._engine.ERROR_AUTHENTICATION ?
                   this.STATE_ERROR_AUTHENTICATION : this.STATE_ERROR_OTHER;
      this._logManager.resetFileLog(this._logManager.REASON_ERROR);
      Services.obs.notifyObservers(null, "readinglist:sync:error", null);
      return intervals.retry;
    }).then(nextDelay => {
      this._timerRunning = false;
      // ensure a new timer is setup for the appropriate next time.
      this._maybeReschedule(nextDelay);
      this._setupTimer();
      this._onAutoReschedule(); // just for tests...
    }).catch(err => {
      // We should never get here, but better safe than sorry...
      this.log.error("Failed to reschedule after sync completed", err);
    });
  },

  _clearTimer() {
    if (this._timer) {
      clearTimeout(this._timer);
      this._timer = null;
    }
  },

  // A function to "sync now", but not allowing it to start if one is
  // already running, and rescheduling the timer.
  // To call this, just send a "readinglist:user-sync" notification.
  _syncNow() {
    if (this._timerRunning) {
      this.log.info("syncNow() but a sync is already in progress - ignoring");
      return;
    }
    this._clearTimer();
    this._doSync();
  },

  // A couple of hook-points for testing.
  // xpcshell tests hook this so (a) it can check the expected delay is set
  // and (b) to ignore the delay and set a timeout of 0 so the test is fast.
  _setTimeout(delay) {
    return setTimeout(() => this._doSync(), delay);
  },
  // xpcshell tests hook this to make sure that the correct state etc exist
  // after a sync has been completed and a new timer created (or not).
  _onAutoReschedule() {},
};

let internalScheduler = new InternalScheduler();
internalScheduler.init();

// The public interface into this module is tiny, so a simple object that
// delegates to the implementation.
let ReadingListScheduler = {
  get STATE_OK() internalScheduler.STATE_OK,
  get STATE_ERROR_AUTHENTICATION() internalScheduler.STATE_ERROR_AUTHENTICATION,
  get STATE_ERROR_OTHER() internalScheduler.STATE_ERROR_OTHER,

  get state() internalScheduler.state,
};

// These functions are exposed purely for tests, which manage to grab them
// via a BackstagePass.
function createTestableScheduler() {
  // kill the "real" scheduler as we don't want it listening to notifications etc.
  if (internalScheduler) {
    internalScheduler.finalize();
    internalScheduler = null;
  }
  // No .init() call - that's up to the tests after hooking.
  return new InternalScheduler();
}

// mochitests want the internal state of the real scheduler for various things.
function getInternalScheduler() {
  return internalScheduler;
}
