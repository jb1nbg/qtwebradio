#include "blctrl.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QList>
#include <QBluetoothSocket>

blctrl::blctrl()
    : QObject ()
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    localDevice = new QBluetoothLocalDevice();

    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(addDevice(QBluetoothDeviceInfo)));
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));
    connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(scanError(QBluetoothDeviceDiscoveryAgent::Error)));

    connect(localDevice, SIGNAL(deviceConnected(QBluetoothAddress)), this, SLOT(deviceConnected(QBluetoothAddress)));
    connect(localDevice, SIGNAL(deviceDisconnected(QBluetoothAddress)), this, SLOT(deviceDisconnected(QBluetoothAddress)));
    connect(localDevice, SIGNAL(error(QBluetoothLocalDevice::Error)), this, SLOT(error(QBluetoothLocalDevice::Error)));
    connect(localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)), this, SLOT(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    connect(localDevice, SIGNAL(pairingDisplayConfirmation(QBluetoothAddress, QString)), this, SLOT(pairingDisplayConfirmation(QBluetoothAddress, QString)));
    connect(localDevice, SIGNAL(pairingDisplayPinCode(QBluetoothAddress, QString)), this, SLOT(pairingDisplayPinCode(QBluetoothAddress, QString)));
    connect(localDevice, SIGNAL(pairingFinished(QBluetoothAddress, QBluetoothLocalDevice::Pairing)), this, SLOT(pairingFinished(QBluetoothAddress, QBluetoothLocalDevice::Pairing)));

    connectSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    connect(connectSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(connectSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(connectSocket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socketError(QBluetoothSocket::SocketError)));
    connect(connectSocket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(socketStateChanged(QBluetoothSocket::SocketState)));

}

blctrl::~blctrl()
{
}

void blctrl::power(bool enabled)
{
    if (enabled)
        localDevice->powerOn();
    else
        localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void blctrl::discovery(bool enabled)
{
    if (enabled)
        discoveryAgent->start();
    else
        discoveryAgent->stop();
}

void blctrl::pair(QBluetoothAddress address)
{
    localDevice->requestPairing(address, QBluetoothLocalDevice::Paired);
}

void blctrl::connectService(QBluetoothAddress address, QBluetoothUuid service)
{
    if (!connectServiceBusy())
        this->connectSocket->connectToService(address, service);
    else
        qDebug("Busy");
}

void blctrl::disconnectService()
{
    qDebug() << this->connectSocket->state();
    this->connectSocket->disconnectFromService();
}

bool blctrl::connectServiceBusy()
{
    return (this->connectSocket->state() != QBluetoothSocket::UnconnectedState);
}

bool blctrl::isDeviceConnected(QBluetoothAddress address)
{    
    QList<QBluetoothAddress> devices = localDevice->connectedDevices();
    qDebug() << "isDeviceConnected" << devices.count();

    for (int i = 0; i < devices.count(); i++)    
    {
        qDebug() << "isDeviceConnected" << i << devices.at(i);
        if (devices.at(i) == address)
            return true;
    }
    return false;
}

QBluetoothLocalDevice::Pairing blctrl::pairingState(QBluetoothAddress address)
{
    return localDevice->pairingStatus(address);
}

void blctrl::addDevice(const QBluetoothDeviceInfo& devInfo)
{
    qDebug() << "DEBUG: addDevice" << devInfo.name() << devInfo.address() << devInfo.isValid() << devInfo.isCached();
}

void blctrl::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "DEBUG: scanError" << error;
}

void blctrl::scanFinished()
{
    qDebug() << "DEBUG: Scan finish";
}

void blctrl::deviceConnected(const QBluetoothAddress &address)
{
    qDebug() << "DEBUG: deviceConnected" << address;
}

void blctrl::deviceDisconnected(const QBluetoothAddress &address)
{
    qDebug() << "DEBUG: deviceDisconnected" << address;
}

void blctrl::error(QBluetoothLocalDevice::Error error)
{
    qDebug() << "DEBUG: error" << error;
}

void blctrl::hostModeStateChanged(QBluetoothLocalDevice::HostMode hostMode)
{
    qDebug() << "DEBUG: hostModeStateChanged" << hostMode;
}

void blctrl::pairingDisplayConfirmation(const QBluetoothAddress &address, QString pin)
{
    qDebug() << "DEBUG: pairingDisplayConfirmation" << address;
    qDebug() << "DEBUG: pairingDisplayConfirmation" << pin;
}

void blctrl::pairingDisplayPinCode(const QBluetoothAddress &address, QString pin)
{
    qDebug() << "DEBUG: pairingDisplayPinCode" << address;
    qDebug() << "DEBUG: pairingDisplayPinCode" << pin;
}

void blctrl::pairingFinished(const QBluetoothAddress& address, QBluetoothLocalDevice::Pairing pairing)
{
    qDebug() << "DEBUG: pairingFinished" << address;
    qDebug() << "DEBUG: pairingFinished" << pairing;
}

void blctrl::socketConnected()
{
    qDebug() << "DEBUG: socketConnected";
}

void blctrl::socketDisconnected()
{
    qDebug() << "DEBUG: socketDisconnected";
}

void blctrl::socketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "DEBUG: socketError" << error;
}

void blctrl::socketStateChanged(QBluetoothSocket::SocketState state)
{
    qDebug() << "DEBUG: socketStateChanged" << state;
}
