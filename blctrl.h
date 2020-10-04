#ifndef CTRL_H
#define CTRL_H

#include <QObject>
#include <QList>

#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothSocket>

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceDiscoveryAgent)
QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)

class blctrl : public QObject
{
    Q_OBJECT

public:
    blctrl();
    ~blctrl();

    QList<QBluetoothAddress> connectedDevices() { return localDevice->connectedDevices(); }
    QList<QBluetoothDeviceInfo> discoveredDevices() { return discoveryAgent->discoveredDevices(); }
    void power(bool enabled);
    bool powerMode() { return localDevice->hostMode() != QBluetoothLocalDevice::HostPoweredOff; }
    void discovery(bool enabled);
    bool discoveryMode() { return discoveryAgent->isActive(); }
    void pair(QBluetoothAddress address);
    QBluetoothLocalDevice::Pairing pairingState(QBluetoothAddress address);
    void connectService(QBluetoothAddress address, QBluetoothUuid service);
    void disconnectService();
    bool connectServiceBusy();
    bool isDeviceConnected(QBluetoothAddress address);
private:
    QBluetoothDeviceDiscoveryAgent* discoveryAgent;
    QBluetoothLocalDevice* localDevice;

private slots:
    void addDevice(const QBluetoothDeviceInfo&);
    void scanFinished();
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);

    void deviceConnected(const QBluetoothAddress &address);
    void deviceDisconnected(const QBluetoothAddress &address);
    void error(QBluetoothLocalDevice::Error error);
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode hostMode);
    void pairingDisplayConfirmation(const QBluetoothAddress &address, QString pin);
    void pairingDisplayPinCode(const QBluetoothAddress &address, QString pin);
    void pairingFinished(const QBluetoothAddress& address, QBluetoothLocalDevice::Pairing pairing);

    void socketConnected();
    void socketDisconnected();
    void socketError(QBluetoothSocket::SocketError error);
    void socketStateChanged(QBluetoothSocket::SocketState state);

private:
    QBluetoothSocket* connectSocket;
};

#endif // CTRL_H
