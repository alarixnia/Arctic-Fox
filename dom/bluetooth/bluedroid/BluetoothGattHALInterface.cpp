/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BluetoothGattHALInterface.h"
#include "BluetoothHALHelpers.h"

BEGIN_BLUETOOTH_NAMESPACE

typedef
  BluetoothHALInterfaceRunnable0<BluetoothGattClientResultHandler, void>
  BluetoothGattClientHALResultRunnable;

typedef
  BluetoothHALInterfaceRunnable1<BluetoothGattClientResultHandler, void,
                                 BluetoothStatus, BluetoothStatus>
  BluetoothGattClientHALErrorRunnable;

typedef
  BluetoothHALInterfaceRunnable0<BluetoothGattServerResultHandler, void>
  BluetoothGattServerHALResultRunnable;

typedef
  BluetoothHALInterfaceRunnable1<BluetoothGattServerResultHandler, void,
                                 BluetoothStatus, BluetoothStatus>
  BluetoothGattServerHALErrorRunnable;

typedef
  BluetoothHALInterfaceRunnable0<BluetoothGattResultHandler, void>
  BluetoothGattHALResultRunnable;

typedef
  BluetoothHALInterfaceRunnable1<BluetoothGattResultHandler, void,
                                 BluetoothStatus, BluetoothStatus>
  BluetoothGattHALErrorRunnable;

static nsresult
DispatchBluetoothGattClientHALResult(
  BluetoothGattClientResultHandler* aRes,
  void (BluetoothGattClientResultHandler::*aMethod)(),
  BluetoothStatus aStatus)
{
  MOZ_ASSERT(aRes);

  nsRunnable* runnable;

  if (aStatus == STATUS_SUCCESS) {
    runnable = new BluetoothGattClientHALResultRunnable(aRes, aMethod);
  } else {
    runnable = new BluetoothGattClientHALErrorRunnable(aRes,
      &BluetoothGattClientResultHandler::OnError, aStatus);
  }
  nsresult rv = NS_DispatchToMainThread(runnable);
  if (NS_FAILED(rv)) {
    BT_WARNING("NS_DispatchToMainThread failed: %X", rv);
  }
  return rv;
}

static nsresult
DispatchBluetoothGattServerHALResult(
  BluetoothGattServerResultHandler* aRes,
  void (BluetoothGattServerResultHandler::*aMethod)(),
  BluetoothStatus aStatus)
{
  MOZ_ASSERT(aRes);

  nsRunnable* runnable;

  if (aStatus == STATUS_SUCCESS) {
    runnable = new BluetoothGattServerHALResultRunnable(aRes, aMethod);
  } else {
    runnable = new BluetoothGattServerHALErrorRunnable(aRes,
      &BluetoothGattServerResultHandler::OnError, aStatus);
  }
  nsresult rv = NS_DispatchToMainThread(runnable);
  if (NS_FAILED(rv)) {
    BT_WARNING("NS_DispatchToMainThread failed: %X", rv);
  }
  return rv;
}

static nsresult
DispatchBluetoothGattHALResult(
  BluetoothGattResultHandler* aRes,
  void (BluetoothGattResultHandler::*aMethod)(),
  BluetoothStatus aStatus)
{
  MOZ_ASSERT(aRes);

  nsRunnable* runnable;

  if (aStatus == STATUS_SUCCESS) {
    runnable = new BluetoothGattHALResultRunnable(aRes, aMethod);
  } else {
    runnable = new BluetoothGattHALErrorRunnable(aRes,
      &BluetoothGattResultHandler::OnError, aStatus);
  }
  nsresult rv = NS_DispatchToMainThread(runnable);
  if (NS_FAILED(rv)) {
    BT_WARNING("NS_DispatchToMainThread failed: %X", rv);
  }
  return rv;
}

// Notification Handling
//

static BluetoothGattNotificationHandler* sGattNotificationHandler;

struct BluetoothGattClientCallback
{
  class GattClientNotificationHandlerWrapper
  {
  public:
    typedef BluetoothGattClientNotificationHandler ObjectType;

    static ObjectType* GetInstance()
    {
      MOZ_ASSERT(NS_IsMainThread());

      return sGattNotificationHandler;
    }
  };

  // Notifications

  // GATT Client Notification
  typedef BluetoothNotificationHALRunnable3<
    GattClientNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, BluetoothUuid,
    BluetoothGattStatus, int, const BluetoothUuid&>
    RegisterClientNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattClientNotificationHandlerWrapper, void,
    nsString, int, BluetoothGattAdvData,
    const nsAString&, int, const BluetoothGattAdvData&>
    ScanResultNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, int, nsString,
    int, BluetoothGattStatus, int, const nsAString&>
    ConnectNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, int, nsString,
    int, BluetoothGattStatus, int, const nsAString&>
    DisconnectNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus>
    SearchCompleteNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattServiceId,
    int, const BluetoothGattServiceId&>
    SearchResultNotification;

  typedef BluetoothNotificationHALRunnable5<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattServiceId,
    BluetoothGattId, BluetoothGattCharProp,
    int, BluetoothGattStatus, const BluetoothGattServiceId&,
    const BluetoothGattId&, const BluetoothGattCharProp&>
    GetCharacteristicNotification;

  typedef BluetoothNotificationHALRunnable5<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattServiceId,
    BluetoothGattId, BluetoothGattId,
    int, BluetoothGattStatus, const BluetoothGattServiceId&,
    const BluetoothGattId&, const BluetoothGattId&>
    GetDescriptorNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattServiceId, BluetoothGattServiceId,
    int, BluetoothGattStatus, const BluetoothGattServiceId&,
    const BluetoothGattServiceId&>
    GetIncludedServiceNotification;

  typedef BluetoothNotificationHALRunnable5<
    GattClientNotificationHandlerWrapper, void,
    int, int, BluetoothGattStatus,
    BluetoothGattServiceId, BluetoothGattId,
    int, int, BluetoothGattStatus,
    const BluetoothGattServiceId&, const BluetoothGattId&>
    RegisterNotificationNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattNotifyParam,
    int, const BluetoothGattNotifyParam&>
    NotifyNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattReadParam,
    int, BluetoothGattStatus, const BluetoothGattReadParam&>
    ReadCharacteristicNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattWriteParam,
    int, BluetoothGattStatus, const BluetoothGattWriteParam&>
    WriteCharacteristicNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattReadParam,
    int, BluetoothGattStatus, const BluetoothGattReadParam&>
    ReadDescriptorNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus, BluetoothGattWriteParam,
    int, BluetoothGattStatus, const BluetoothGattWriteParam&>
    WriteDescriptorNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattClientNotificationHandlerWrapper, void,
    int, BluetoothGattStatus>
    ExecuteWriteNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattClientNotificationHandlerWrapper, void,
    int, nsString, int, BluetoothGattStatus,
    int, const nsAString&, int, BluetoothGattStatus>
    ReadRemoteRssiNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattClientNotificationHandlerWrapper, void,
    BluetoothGattStatus, int>
    ListenNotification;

  // Bluedroid GATT client callbacks
#if ANDROID_VERSION >= 19
  static void
  RegisterClient(int aStatus, int aClientIf, bt_uuid_t* aAppUuid)
  {
    RegisterClientNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::RegisterClientNotification,
      aStatus, aClientIf, *aAppUuid);
  }

  static void
  ScanResult(bt_bdaddr_t* aBdAddr, int aRssi, uint8_t* aAdvData)
  {
    ScanResultNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ScanResultNotification,
      aBdAddr, aRssi, aAdvData);
  }

  static void
  Connect(int aConnId, int aStatus, int aClientIf, bt_bdaddr_t* aBdAddr)
  {
    ConnectNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ConnectNotification,
      aConnId, aStatus, aClientIf, aBdAddr);
  }

  static void
  Disconnect(int aConnId, int aStatus, int aClientIf, bt_bdaddr_t* aBdAddr)
  {
    DisconnectNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::DisconnectNotification,
      aConnId, aStatus, aClientIf, aBdAddr);
  }

  static void
  SearchComplete(int aConnId, int aStatus)
  {
    SearchCompleteNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::SearchCompleteNotification,
      aConnId, aStatus);
  }

  static void
  SearchResult(int aConnId, btgatt_srvc_id_t* aServiceId)
  {
    SearchResultNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::SearchResultNotification,
      aConnId, *aServiceId);
  }

  static void
  GetCharacteristic(int aConnId, int aStatus,
                    btgatt_srvc_id_t* aServiceId,
                    btgatt_gatt_id_t* aCharId,
                    int aCharProperty)
  {
    GetCharacteristicNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::GetCharacteristicNotification,
      aConnId, aStatus, *aServiceId, *aCharId, aCharProperty);
  }

  static void
  GetDescriptor(int aConnId, int aStatus,
                btgatt_srvc_id_t* aServiceId,
                btgatt_gatt_id_t* aCharId,
                btgatt_gatt_id_t* aDescriptorId)
  {
    GetDescriptorNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::GetDescriptorNotification,
      aConnId, aStatus, *aServiceId, *aCharId, *aDescriptorId);
  }

  static void
  GetIncludedService(int aConnId, int aStatus,
                     btgatt_srvc_id_t* aServiceId,
                     btgatt_srvc_id_t* aIncludedServiceId)
  {
    GetIncludedServiceNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::GetIncludedServiceNotification,
      aConnId, aStatus, *aServiceId, *aIncludedServiceId);
  }

  static void
  RegisterNotification(int aConnId, int aIsRegister, int aStatus,
                       btgatt_srvc_id_t* aServiceId,
                       btgatt_gatt_id_t* aCharId)
  {
    RegisterNotificationNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::RegisterNotificationNotification,
      aConnId, aIsRegister, aStatus, *aServiceId, *aCharId);
  }

  static void
  Notify(int aConnId, btgatt_notify_params_t* aParam)
  {
    NotifyNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::NotifyNotification,
      aConnId, *aParam);
  }

  static void
  ReadCharacteristic(int aConnId, int aStatus, btgatt_read_params_t* aParam)
  {
    ReadCharacteristicNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ReadCharacteristicNotification,
      aConnId, aStatus, *aParam);
  }

  static void
  WriteCharacteristic(int aConnId, int aStatus, btgatt_write_params_t* aParam)
  {
    WriteCharacteristicNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::WriteCharacteristicNotification,
      aConnId, aStatus, *aParam);
  }

  static void
  ReadDescriptor(int aConnId, int aStatus, btgatt_read_params_t* aParam)
  {
    ReadDescriptorNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ReadDescriptorNotification,
      aConnId, aStatus, *aParam);
  }

  static void
  WriteDescriptor(int aConnId, int aStatus, btgatt_write_params_t* aParam)
  {
    WriteDescriptorNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::WriteDescriptorNotification,
      aConnId, aStatus, *aParam);
  }

  static void
  ExecuteWrite(int aConnId, int aStatus)
  {
    ExecuteWriteNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ExecuteWriteNotification,
      aConnId, aStatus);
  }

  static void
  ReadRemoteRssi(int aClientIf, bt_bdaddr_t* aBdAddr, int aRssi, int aStatus)
  {
    ReadRemoteRssiNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ReadRemoteRssiNotification,
      aClientIf, aBdAddr, aRssi, aStatus);
  }

  static void
  Listen(int aStatus, int aServerIf)
  {
    ListenNotification::Dispatch(
      &BluetoothGattClientNotificationHandler::ListenNotification,
      aStatus, aServerIf);
  }
#endif // ANDROID_VERSION >= 19
};

struct BluetoothGattServerCallback
{
  class GattServerNotificationHandlerWrapper
  {
  public:
    typedef BluetoothGattServerNotificationHandler ObjectType;

    static ObjectType* GetInstance()
    {
      MOZ_ASSERT(NS_IsMainThread());

      return sGattNotificationHandler;
    }
  };

  // Notifications
  typedef BluetoothNotificationHALRunnable3<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, BluetoothUuid,
    BluetoothGattStatus, int, const BluetoothUuid&>
    RegisterServerNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattServerNotificationHandlerWrapper, void,
    int, int, bool, nsString,
    int, int, bool, const nsAString&>
    ConnectionNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, BluetoothGattServiceId, int,
    BluetoothGattStatus, int, const BluetoothGattServiceId&, int>
    ServiceAddedNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, int, int>
    IncludedServiceAddedNotification;

  typedef BluetoothNotificationHALRunnable5<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, BluetoothUuid, int, int,
    BluetoothGattStatus, int, const BluetoothUuid&, int, int>
    CharacteristicAddedNotification;

  typedef BluetoothNotificationHALRunnable5<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, BluetoothUuid, int, int,
    BluetoothGattStatus, int, const BluetoothUuid&, int, int>
    DescriptorAddedNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, int>
    ServiceStartedNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, int>
    ServiceStoppedNotification;

  typedef BluetoothNotificationHALRunnable3<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int, int>
    ServiceDeletedNotification;

  typedef BluetoothNotificationHALRunnable6<
    GattServerNotificationHandlerWrapper, void,
    int, int, nsString, int, int, bool,
    int, int, const nsAString&, int, int, bool>
    RequestReadNotification;

  typedef BluetoothNotificationHALRunnable8<
    GattServerNotificationHandlerWrapper, void,
    int, int, nsString, int, int, nsTArray<uint8_t>, bool, bool,
    int, int, const nsAString&, int, int, const nsTArray<uint8_t>&, bool, bool>
    RequestWriteNotification;

  typedef BluetoothNotificationHALRunnable4<
    GattServerNotificationHandlerWrapper, void,
    int, int, nsString, bool,
    int, int, const nsAString&, bool>
    RequestExecuteWriteNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattServerNotificationHandlerWrapper, void,
    BluetoothGattStatus, int>
    ResponseConfirmationNotification;

#if ANDROID_VERSION >= 21
  typedef BluetoothNotificationHALRunnable2<
    GattServerNotificationHandlerWrapper, void,
    int, BluetoothGattStatus>
    IndicationSentNotification;

  typedef BluetoothNotificationHALRunnable2<
    GattServerNotificationHandlerWrapper, void,
    int, bool>
    CongestionNotification;
#endif // ANDROID_VERSION >= 21

#if ANDROID_VERSION >=22
  typedef BluetoothNotificationHALRunnable2<
    GattServerNotificationHandlerWrapper, void,
    int, int>
    MtuChangedNotification;
#endif // ANDROID_VERSION >=22

  // GATT Server callbacks
#if ANDROID_VERSION >= 19
  static void
  RegisterServer(int aStatus, int aServerIf, bt_uuid_t* aAppUuid)
  {
    RegisterServerNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::RegisterServerNotification,
      aStatus, aServerIf, *aAppUuid);
  }

  static void
  Connection(int aConnId, int aServerIf, int aIsConnected,
             bt_bdaddr_t* aBdAddr)
  {
    ConnectionNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::ConnectionNotification,
      aConnId, aServerIf, aIsConnected != 0, aBdAddr);
  }

  static void
  ServiceAdded(int aStatus, int aServerIf, btgatt_srvc_id_t* aServiceId,
               int aServiceHandle)
  {
    ServiceAddedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::ServiceAddedNotification,
      aStatus, aServerIf, *aServiceId, aServiceHandle);
  }

  static void
  IncludedServiceAdded(int aStatus, int aServerIf, int aServiceHandle,
                       int aIncludedServiceHandle)
  {
    IncludedServiceAddedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::IncludedServiceAddedNotification,
      aStatus, aServerIf, aServiceHandle, aIncludedServiceHandle);
  }

  static void
  CharacteristicAdded(int aStatus, int aServerIf, bt_uuid_t* aUuid,
                      int aServiceHandle, int aCharHandle)
  {
    CharacteristicAddedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::CharacteristicAddedNotification,
      aStatus, aServerIf, *aUuid, aServiceHandle, aCharHandle);
  }

  static void
  DescriptorAdded(int aStatus, int aServerIf, bt_uuid_t* aUuid,
                  int aServiceHandle, int aDescriptorHandle)
  {
    DescriptorAddedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::DescriptorAddedNotification,
      aStatus, aServerIf, *aUuid, aServiceHandle, aDescriptorHandle);
  }

  static void
  ServiceStarted(int aStatus, int aServerIf, int aServiceHandle)
  {
    ServiceStartedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::ServiceStartedNotification,
      aStatus, aServerIf, aServiceHandle);
  }

  static void
  ServiceStopped(int aStatus, int aServerIf, int aServiceHandle)
  {
    ServiceStoppedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::ServiceStoppedNotification,
      aStatus, aServerIf, aServiceHandle);
  }

  static void
  ServiceDeleted(int aStatus, int aServerIf, int aServiceHandle)
  {
    ServiceDeletedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::ServiceDeletedNotification,
      aStatus, aServerIf, aServiceHandle);
  }

  static void
  RequestRead(int aConnId, int aTransId, bt_bdaddr_t* aBdAddr,
              int aAttrHandle, int aOffset, bool aIsLong)
  {
    RequestReadNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::RequestReadNotification,
      aConnId, aTransId, *aBdAddr, aAttrHandle, aOffset, aIsLong);
  }

  static void
  RequestWrite(int aConnId, int aTransId, bt_bdaddr_t* aBdAddr,
               int aAttrHandle, int aOffset, int aLength,
               bool aNeedRsp, bool aIsPrep, uint8_t* aValue)
  {
    nsTArray<uint8_t> value;
    value.AppendElements(aValue, aLength);
    RequestWriteNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::RequestWriteNotification,
      aConnId, aTransId, *aBdAddr, aAttrHandle, aOffset, value, aNeedRsp,
      aIsPrep);
  }

  static void
  RequestExecuteWrite(int aConnId, int aTransId, bt_bdaddr_t* aBdAddr,
                      int aExecWrite)
  {
    RequestExecuteWriteNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::RequestExecuteWriteNotification,
      aConnId, aTransId, *aBdAddr, aExecWrite != 0);
  }

  static void
  ResponseConfirmation(int aStatus, int aHandle)
  {
    ResponseConfirmationNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::ResponseConfirmationNotification,
      aStatus, aHandle);
  }
#endif // ANDROID_VERSION >= 19

#if ANDROID_VERSION >= 21
  static void
  IndicationSent(int aConnId, int aStatus)
  {
    IndicationSentNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::IndicationSentNotification,
      aConnId, aStatus);
  }

  static void
  Congestion(int aConnId, bool aCongested)
  {
    CongestionNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::CongestionNotification,
      aConnId, aCongested);
  }
#endif // ANDROID_VERSION >= 21

#if ANDROID_VERSION >= 22
  static void
  MtuChanged(int aConnId, int aMtu)
  {
    MtuChangedNotification::Dispatch(
      &BluetoothGattServerNotificationHandler::MtuChangedNotification,
      aConnId, aMtu);
  }
#endif // ANDROID_VERSION >= 22
};

// GATT Client Interface

BluetoothGattClientHALInterface::BluetoothGattClientHALInterface(
#if ANDROID_VERSION >= 19
  const btgatt_client_interface_t* aInterface
#endif
  )
#if ANDROID_VERSION >= 19
  :mInterface(aInterface)
#endif
{
#if ANDROID_VERSION >= 19
  MOZ_ASSERT(mInterface);
#endif
}

BluetoothGattClientHALInterface::~BluetoothGattClientHALInterface()
{ }

void
BluetoothGattClientHALInterface::RegisterClient(
  const BluetoothUuid& aUuid, BluetoothGattClientResultHandler* aRes)
{
  int status;

#if ANDROID_VERSION >= 19
  bt_uuid_t uuid;
  if (NS_SUCCEEDED(Convert(aUuid, uuid))) {
    status = mInterface->register_client(&uuid);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif
  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::RegisterClient,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::UnregisterClient(
  int aClientIf, BluetoothGattClientResultHandler* aRes)
{
#if ANDROID_VERSION >= 19
  int status = mInterface->unregister_client(aClientIf);
#else
  int status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::UnregisterClient,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::Scan(
  int aClientIf, bool aStart, BluetoothGattClientResultHandler* aRes)
{
#if ANDROID_VERSION >= 21
  int status = mInterface->scan(aStart);
#elif ANDROID_VERSION >= 19
  int status = mInterface->scan(aClientIf, aStart);
#else
  int status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::Scan,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::Connect(
  int aClientIf, const nsAString& aBdAddr,
  bool aIsDirect, BluetoothTransport aTransport,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 21
  bt_bdaddr_t bdAddr;
  btgatt_transport_t transport;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr)) ||
      NS_SUCCEEDED(Convert(aTransport, transport))) {
    status = mInterface->connect(aClientIf, &bdAddr, aIsDirect, transport);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#elif ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->connect(aClientIf, &bdAddr, aIsDirect);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::Connect,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::Disconnect(
  int aClientIf, const nsAString& aBdAddr,
  int aConnId, BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->disconnect(aClientIf, &bdAddr, aConnId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::Disconnect,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::Listen(
  int aClientIf, bool aIsStart, BluetoothGattClientResultHandler* aRes)
{
#if ANDROID_VERSION >= 19
  bt_status_t status = mInterface->listen(aClientIf, aIsStart);
#else
  bt_status_t status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::Listen,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::Refresh(
  int aClientIf, const nsAString& aBdAddr,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->refresh(aClientIf, &bdAddr);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::Refresh,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::SearchService(
  int aConnId, bool aSearchAll, const BluetoothUuid& aUuid,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_uuid_t uuid;

  if (aSearchAll) {
    status = mInterface->search_service(aConnId, 0);
  } else if (NS_SUCCEEDED(Convert(aUuid, uuid))) {
    status = mInterface->search_service(aConnId, &uuid);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::SearchService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::GetIncludedService(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  bool aFirst, const BluetoothGattServiceId& aStartServiceId,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_srvc_id_t startServiceId;

  if (aFirst && NS_SUCCEEDED(Convert(aServiceId, serviceId))) {
    status = mInterface->get_included_service(aConnId, &serviceId, 0);
  } else if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
             NS_SUCCEEDED(Convert(aStartServiceId, startServiceId))) {
    status = mInterface->get_included_service(aConnId, &serviceId,
                                              &startServiceId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::GetIncludedService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::GetCharacteristic(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  bool aFirst, const BluetoothGattId& aStartCharId,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t startCharId;

  if (aFirst && NS_SUCCEEDED(Convert(aServiceId, serviceId))) {
    status = mInterface->get_characteristic(aConnId, &serviceId, 0);
  } else if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
             NS_SUCCEEDED(Convert(aStartCharId, startCharId))) {
    status = mInterface->get_characteristic(aConnId, &serviceId, &startCharId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::GetCharacteristic,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::GetDescriptor(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId, bool aFirst,
  const BluetoothGattId& aDescriptorId, BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;
  btgatt_gatt_id_t descriptorId;

  if (aFirst &&
      NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId))) {
    status = mInterface->get_descriptor(aConnId, &serviceId, &charId, 0);
  } else if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
             NS_SUCCEEDED(Convert(aCharId, charId)) &&
             NS_SUCCEEDED(Convert(aDescriptorId, descriptorId))) {
    status = mInterface->get_descriptor(aConnId, &serviceId, &charId,
                                        &descriptorId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::GetDescriptor,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::ReadCharacteristic(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId, BluetoothGattAuthReq aAuthReq,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;
  int authReq;

  if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId)) &&
      NS_SUCCEEDED(Convert(aAuthReq, authReq))) {
    status = mInterface->read_characteristic(aConnId, &serviceId, &charId,
                                             authReq);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::ReadCharacteristic,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::WriteCharacteristic(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId, BluetoothGattWriteType aWriteType,
  BluetoothGattAuthReq aAuthReq, const nsTArray<uint8_t>& aValue,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;
  int writeType;
  int authReq;

  if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId)) &&
      NS_SUCCEEDED(Convert(aWriteType, writeType)) &&
      NS_SUCCEEDED(Convert(aAuthReq, authReq))) {
    status = mInterface->write_characteristic(
      aConnId, &serviceId, &charId, writeType,
      aValue.Length() * sizeof(uint8_t), authReq,
      reinterpret_cast<char*>(const_cast<uint8_t*>(aValue.Elements())));
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::WriteCharacteristic,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::ReadDescriptor(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId,
  const BluetoothGattId& aDescriptorId,
  BluetoothGattAuthReq aAuthReq, BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;
  btgatt_gatt_id_t descriptorId;
  int authReq;

  if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId)) &&
      NS_SUCCEEDED(Convert(aDescriptorId, descriptorId)) &&
      NS_SUCCEEDED(Convert(aAuthReq, authReq))) {
    status = mInterface->read_descriptor(aConnId, &serviceId, &charId,
                                         &descriptorId, authReq);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::ReadDescriptor,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::WriteDescriptor(
  int aConnId, const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId,
  const BluetoothGattId& aDescriptorId,
  BluetoothGattWriteType aWriteType,
  BluetoothGattAuthReq aAuthReq,
  const nsTArray<uint8_t>& aValue,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;
  btgatt_gatt_id_t descriptorId;
  int writeType;
  int authReq;

  if (NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId)) &&
      NS_SUCCEEDED(Convert(aDescriptorId, descriptorId)) &&
      NS_SUCCEEDED(Convert(aWriteType, writeType)) &&
      NS_SUCCEEDED(Convert(aAuthReq, authReq))) {
    status = mInterface->write_descriptor(
      aConnId, &serviceId, &charId, &descriptorId, writeType,
      aValue.Length() * sizeof(uint8_t), authReq,
      reinterpret_cast<char*>(const_cast<uint8_t*>(aValue.Elements())));
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::WriteDescriptor,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::ExecuteWrite(
  int aConnId, int aIsExecute, BluetoothGattClientResultHandler* aRes)
{
#if ANDROID_VERSION >= 19
  int status = mInterface->execute_write(aConnId, aIsExecute);
#else
  int status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::ExecuteWrite,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::RegisterNotification(
  int aClientIf, const nsAString& aBdAddr,
  const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr)) &&
      NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId))) {
    status = mInterface->register_for_notification(aClientIf, &bdAddr,
                                                   &serviceId, &charId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::RegisterNotification,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::DeregisterNotification(
  int aClientIf, const nsAString& aBdAddr,
  const BluetoothGattServiceId& aServiceId,
  const BluetoothGattId& aCharId,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;
  btgatt_srvc_id_t serviceId;
  btgatt_gatt_id_t charId;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr)) &&
      NS_SUCCEEDED(Convert(aServiceId, serviceId)) &&
      NS_SUCCEEDED(Convert(aCharId, charId))) {
    status = mInterface->deregister_for_notification(aClientIf, &bdAddr,
                                                     &serviceId, &charId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::DeregisterNotification,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::ReadRemoteRssi(
  int aClientIf, const nsAString& aBdAddr,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->read_remote_rssi(aClientIf, &bdAddr);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::ReadRemoteRssi,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::GetDeviceType(
  const nsAString& aBdAddr, BluetoothGattClientResultHandler* aRes)
{
  int status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->get_device_type(&bdAddr);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::GetDeviceType,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::SetAdvData(
  int aServerIf, bool aIsScanRsp, bool aIsNameIncluded,
  bool aIsTxPowerIncluded, int aMinInterval, int aMaxInterval, int aApperance,
  uint16_t aManufacturerLen, char* aManufacturerData,
  uint16_t aServiceDataLen, char* aServiceData,
  uint16_t aServiceUUIDLen, char* aServiceUUID,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 21
  status = mInterface->set_adv_data(
    aServerIf, aIsScanRsp, aIsNameIncluded, aIsTxPowerIncluded,
    aMinInterval, aMaxInterval, aApperance,
    aManufacturerLen, aManufacturerData,
    aServiceDataLen, aServiceData, aServiceUUIDLen, aServiceUUID);
#elif ANDROID_VERSION >= 19
  status = mInterface->set_adv_data(
    aServerIf, aIsScanRsp, aIsNameIncluded, aIsTxPowerIncluded, aMinInterval,
    aMaxInterval, aApperance, aManufacturerLen, aManufacturerData);
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::SetAdvData,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattClientHALInterface::TestCommand(
  int aCommand, const BluetoothGattTestParam& aTestParam,
  BluetoothGattClientResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_test_params_t testParam;

  if (NS_SUCCEEDED(Convert(aTestParam, testParam))) {
    status = mInterface->test_command(aCommand, &testParam);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattClientHALResult(
      aRes, &BluetoothGattClientResultHandler::TestCommand,
      ConvertDefault(status, STATUS_FAIL));
  }

}

// GATT Server Interface

BluetoothGattServerHALInterface::BluetoothGattServerHALInterface(
#if ANDROID_VERSION >= 19
  const btgatt_server_interface_t* aInterface
#endif
  )
#if ANDROID_VERSION >= 19
  :mInterface(aInterface)
#endif
{
#if ANDROID_VERSION >= 19
  MOZ_ASSERT(mInterface);
#endif
}

BluetoothGattServerHALInterface::~BluetoothGattServerHALInterface()
{ }

void
BluetoothGattServerHALInterface::RegisterServer(
  const BluetoothUuid& aUuid, BluetoothGattServerResultHandler* aRes)
{
  int status;
#if ANDROID_VERSION >= 19
  bt_uuid_t uuid;
  if (NS_SUCCEEDED(Convert(aUuid, uuid))) {
    status = mInterface->register_server(&uuid);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::RegisterServer,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::UnregisterServer(
  int aServerIf, BluetoothGattServerResultHandler* aRes)
{
#if ANDROID_VERSION >= 19
  int status = mInterface->unregister_server(aServerIf);
#else
  int status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::UnregisterServer,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::ConnectPeripheral(
  int aServerIf, const nsAString& aBdAddr, bool aIsDirect, /* auto connect */
  BluetoothTransport aTransport, BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 21
  bt_bdaddr_t bdAddr;
  btgatt_transport_t transport;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr)) &&
      NS_SUCCEEDED(Convert(aTransport, transport))) {
    status = mInterface->connect(aServerIf, &bdAddr, aIsDirect, transport);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#elif ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->connect(aServerIf, &bdAddr, aIsDirect);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::ConnectPeripheral,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::DisconnectPeripheral(
  int aServerIf, const nsAString& aBdAddr, int aConnId,
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_bdaddr_t bdAddr;

  if (NS_SUCCEEDED(Convert(aBdAddr, bdAddr))) {
    status = mInterface->disconnect(aServerIf, &bdAddr, aConnId);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::DisconnectPeripheral,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::AddService(
  int aServerIf, const BluetoothGattServiceId& aServiceId, int aNumHandles,
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  btgatt_srvc_id_t serviceId;

  if (NS_SUCCEEDED(Convert(aServiceId, serviceId))) {
    status = mInterface->add_service(aServerIf, &serviceId, aNumHandles);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::AddService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::AddIncludedService(
  int aServerIf, int aServiceHandle, int aIncludedServiceHandle,
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  status = mInterface->add_included_service(aServerIf, aServiceHandle,
                                            aIncludedServiceHandle);
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::AddIncludedService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::AddCharacteristic(
  int aServerIf, int aServiceHandle, const BluetoothUuid& aUuid,
  BluetoothGattCharProp aProperties, BluetoothGattAttrPerm aPermissions,
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_uuid_t uuid;
  int properties;
  int permissions;

  if (NS_SUCCEEDED(Convert(aUuid, uuid)) &&
      NS_SUCCEEDED(Convert(aProperties, properties)) &&
      NS_SUCCEEDED(Convert(aPermissions, permissions))) {
    status = mInterface->add_characteristic(aServerIf, aServiceHandle, &uuid,
                                            properties, permissions);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::AddCharacteristic,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::AddDescriptor(
  int aServerIf, int aServiceHandle, const BluetoothUuid& aUuid,
  BluetoothGattAttrPerm aPermissions, BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  bt_uuid_t uuid;
  int permissions;

  if (NS_SUCCEEDED(Convert(aUuid, uuid)) &&
      NS_SUCCEEDED(Convert(aPermissions, permissions))) {
    status = mInterface->add_descriptor(aServerIf, aServiceHandle, &uuid,
                                        permissions);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::AddDescriptor,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::StartService(
  int aServerIf, int aServiceHandle, BluetoothTransport aTransport,
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  int transport;

  if (NS_SUCCEEDED(Convert(aTransport, transport))) {
    status = mInterface->start_service(aServerIf, aServiceHandle, transport);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::StartService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::StopService(
  int aServerIf, int aServiceHandle, BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  status = mInterface->stop_service(aServerIf, aServiceHandle);
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::StopService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::DeleteService(
  int aServerIf, int aServiceHandle, BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  status = mInterface->delete_service(aServerIf, aServiceHandle);
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::DeleteService,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::SendIndication(
  int aServerIf, int aAttributeHandle, int aConnId,
  const nsTArray<uint8_t>& aValue,
  bool aConfirm, /* true: indication, false: notification */
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  char* value =
    reinterpret_cast<char*>(const_cast<uint8_t*>(aValue.Elements()));
  status = mInterface->send_indication(aServerIf, aAttributeHandle, aConnId,
                                       aValue.Length(), aConfirm, value);
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::SendIndication,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattServerHALInterface::SendResponse(
  int aConnId, int aTransId, BluetoothGattStatus aStatus,
  const BluetoothGattResponse& aResponse,
  BluetoothGattServerResultHandler* aRes)
{
  bt_status_t status;
#if ANDROID_VERSION >= 19
  int response_status;
  btgatt_response_t response;
  if (NS_SUCCEEDED(Convert(aStatus, response_status)) &&
      NS_SUCCEEDED(Convert(aResponse, response))) {
    status = mInterface->send_response(aConnId, aTransId, response_status,
                                       &response);
  } else {
    status = BT_STATUS_PARM_INVALID;
  }
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattServerHALResult(
      aRes, &BluetoothGattServerResultHandler::SendResponse,
      ConvertDefault(status, STATUS_FAIL));
  }
}

// GATT Interface

BluetoothGattHALInterface::BluetoothGattHALInterface(
#if ANDROID_VERSION >= 19
  const btgatt_interface_t* aInterface
#endif
  )
#if ANDROID_VERSION >= 19
  :mInterface(aInterface)
#endif
{
#if ANDROID_VERSION >= 19
  MOZ_ASSERT(mInterface);
#endif
}

BluetoothGattHALInterface::~BluetoothGattHALInterface()
{ }

void
BluetoothGattHALInterface::Init(
  BluetoothGattNotificationHandler* aNotificationHandler,
  BluetoothGattResultHandler* aRes)
{
#if ANDROID_VERSION >= 19
  static const btgatt_client_callbacks_t sGattClientCallbacks = {
    BluetoothGattClientCallback::RegisterClient,
    BluetoothGattClientCallback::ScanResult,
    BluetoothGattClientCallback::Connect,
    BluetoothGattClientCallback::Disconnect,
    BluetoothGattClientCallback::SearchComplete,
    BluetoothGattClientCallback::SearchResult,
    BluetoothGattClientCallback::GetCharacteristic,
    BluetoothGattClientCallback::GetDescriptor,
    BluetoothGattClientCallback::GetIncludedService,
    BluetoothGattClientCallback::RegisterNotification,
    BluetoothGattClientCallback::Notify,
    BluetoothGattClientCallback::ReadCharacteristic,
    BluetoothGattClientCallback::WriteCharacteristic,
    BluetoothGattClientCallback::ReadDescriptor,
    BluetoothGattClientCallback::WriteDescriptor,
    BluetoothGattClientCallback::ExecuteWrite,
    BluetoothGattClientCallback::ReadRemoteRssi,
    BluetoothGattClientCallback::Listen
  };

  static const btgatt_server_callbacks_t sGattServerCallbacks = {
    BluetoothGattServerCallback::RegisterServer,
    BluetoothGattServerCallback::Connection,
    BluetoothGattServerCallback::ServiceAdded,
    BluetoothGattServerCallback::IncludedServiceAdded,
    BluetoothGattServerCallback::CharacteristicAdded,
    BluetoothGattServerCallback::DescriptorAdded,
    BluetoothGattServerCallback::ServiceStarted,
    BluetoothGattServerCallback::ServiceStopped,
    BluetoothGattServerCallback::ServiceDeleted,
    BluetoothGattServerCallback::RequestRead,
    BluetoothGattServerCallback::RequestWrite,
    BluetoothGattServerCallback::RequestExecuteWrite,
    BluetoothGattServerCallback::ResponseConfirmation,
#if ANDROID_VERSION >= 21
    BluetoothGattServerCallback::IndicationSent,
    BluetoothGattServerCallback::Congestion,
#endif // ANDROID_VERSION >= 21
#if ANDROID_VERSION >= 22
    BluetoothGattServerCallback::MtuChanged
#endif // ANDROID_VERSION >= 22
  };

  static const btgatt_callbacks_t sCallbacks = {
    sizeof(sCallbacks),
    &sGattClientCallbacks,
    &sGattServerCallbacks
  };
#endif // ANDROID_VERSION >= 19

  sGattNotificationHandler = aNotificationHandler;

#if ANDROID_VERSION >= 19
  bt_status_t status = mInterface->init(&sCallbacks);
#else
  bt_status_t status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattHALResult(
      aRes, &BluetoothGattResultHandler::Init,
      ConvertDefault(status, STATUS_FAIL));
  }
}

void
BluetoothGattHALInterface::Cleanup(BluetoothGattResultHandler* aRes)
{
  bt_status_t status;

#if ANDROID_VERSION >= 19
  mInterface->cleanup();
  status = BT_STATUS_SUCCESS;
#else
  status = BT_STATUS_UNSUPPORTED;
#endif

  if (aRes) {
    DispatchBluetoothGattHALResult(
      aRes, &BluetoothGattResultHandler::Cleanup,
      ConvertDefault(status, STATUS_FAIL));
  }
}

BluetoothGattClientInterface*
BluetoothGattHALInterface::GetBluetoothGattClientInterface()
{
  static BluetoothGattClientHALInterface* sBluetoothGattClientHALInterface;

  if (sBluetoothGattClientHALInterface) {
    return sBluetoothGattClientHALInterface;
  }

#if ANDROID_VERSION >= 19
  MOZ_ASSERT(mInterface->client);
  sBluetoothGattClientHALInterface =
    new BluetoothGattClientHALInterface(mInterface->client);
#else
  sBluetoothGattClientHALInterface =
    new BluetoothGattClientHALInterface();
#endif

  return sBluetoothGattClientHALInterface;
}

BluetoothGattServerInterface*
BluetoothGattHALInterface::GetBluetoothGattServerInterface()
{
  static BluetoothGattServerHALInterface* sBluetoothGattServerHALInterface;

  if (sBluetoothGattServerHALInterface) {
    return sBluetoothGattServerHALInterface;
  }

#if ANDROID_VERSION >= 19
  MOZ_ASSERT(mInterface->server);
  sBluetoothGattServerHALInterface =
    new BluetoothGattServerHALInterface(mInterface->server);
#else
  sBluetoothGattServerHALInterface =
    new BluetoothGattServerHALInterface();
#endif

  return sBluetoothGattServerHALInterface;
}

END_BLUETOOTH_NAMESPACE
