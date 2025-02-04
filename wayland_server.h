/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef KWIN_WAYLAND_SERVER_H
#define KWIN_WAYLAND_SERVER_H

#include <kwinglobals.h>
#include "keyboard_input.h"

#include <QObject>

class QThread;
class QProcess;
class QWindow;

namespace KWayland
{
namespace Client
{
class ConnectionThread;
class Registry;
class Compositor;
class Seat;
class DataDeviceManager;
class ShmPool;
class Surface;
}
namespace Server
{
class AppMenuManagerInterface;
class ClientConnection;
class CompositorInterface;
class Display;
class DataDeviceInterface;
class IdleInterface;
class SeatInterface;
class DataDeviceManagerInterface;
class ServerSideDecorationManagerInterface;
class ServerSideDecorationPaletteManagerInterface;
class SurfaceInterface;
class OutputInterface;
class PlasmaShellInterface;
class PlasmaShellSurfaceInterface;
class PlasmaVirtualDesktopManagementInterface;
class PlasmaWindowManagementInterface;
class QtSurfaceExtensionInterface;
class OutputManagementInterface;
class OutputConfigurationInterface;
class XdgDecorationManagerInterface;
class XdgShellInterface;
class XdgForeignInterface;
class XdgOutputManagerInterface;
class KeyStateInterface;
}
}

namespace KWin
{
class XdgShellClient;

class AbstractClient;
class Toplevel;

class KWIN_EXPORT WaylandServer : public QObject
{
    Q_OBJECT
public:
    enum class InitalizationFlag {
        NoOptions = 0x0,
        LockScreen = 0x1,
        NoLockScreenIntegration = 0x2,
        NoGlobalShortcuts = 0x4
    };

    Q_DECLARE_FLAGS(InitalizationFlags, InitalizationFlag)

    ~WaylandServer() override;
    bool init(const QByteArray &socketName = QByteArray(), InitalizationFlags flags = InitalizationFlag::NoOptions);
    void terminateClientConnections();

    KWayland::Server::Display *display() {
        return m_display;
    }
    KWayland::Server::CompositorInterface *compositor() {
        return m_compositor;
    }
    KWayland::Server::SeatInterface *seat() {
        return m_seat;
    }
    KWayland::Server::DataDeviceManagerInterface *dataDeviceManager() {
        return m_dataDeviceManager;
    }
    KWayland::Server::PlasmaVirtualDesktopManagementInterface *virtualDesktopManagement() {
        return m_virtualDesktopManagement;
    }
    KWayland::Server::PlasmaWindowManagementInterface *windowManagement() {
        return m_windowManagement;
    }
    KWayland::Server::ServerSideDecorationManagerInterface *decorationManager() const {
        return m_decorationManager;
    }
    KWayland::Server::XdgOutputManagerInterface *xdgOutputManager() const {
        return m_xdgOutputManager;
    }

    QList<XdgShellClient *> clients() const {
        return m_clients;
    }
    void removeClient(XdgShellClient *c);
    XdgShellClient *findClient(quint32 id) const;
    XdgShellClient *findClient(KWayland::Server::SurfaceInterface *surface) const;
    AbstractClient *findAbstractClient(KWayland::Server::SurfaceInterface *surface) const;

    /**
     * @returns a transient parent of a surface imported with the foreign protocol, if any
     */
    KWayland::Server::SurfaceInterface *findForeignTransientForSurface(KWayland::Server::SurfaceInterface *surface);

    /**
     * @returns file descriptor for Xwayland to connect to.
     */
    int createXWaylandConnection();
    void destroyXWaylandConnection();

    /**
     * @returns file descriptor to the input method server's socket.
     */
    int createInputMethodConnection();
    void destroyInputMethodConnection();

    /**
     * @returns true if screen is locked.
     */
    bool isScreenLocked() const;
    /**
     * @returns whether integration with KScreenLocker is available.
     */
    bool hasScreenLockerIntegration() const;

    /**
     * @returns whether any kind of global shortcuts are supported.
     */
    bool hasGlobalShortcutSupport() const;

    void createInternalConnection();
    void initWorkspace();

    KWayland::Server::ClientConnection *xWaylandConnection() const {
        return m_xwayland.client;
    }
    KWayland::Server::ClientConnection *inputMethodConnection() const {
        return m_inputMethodServerConnection;
    }
    KWayland::Server::ClientConnection *internalConnection() const {
        return m_internalConnection.server;
    }
    KWayland::Server::ClientConnection *screenLockerClientConnection() const {
        return m_screenLockerClientConnection;
    }
    KWayland::Client::Compositor *internalCompositor() {
        return m_internalConnection.compositor;
    }
    KWayland::Client::Seat *internalSeat() {
        return m_internalConnection.seat;
    }
    KWayland::Client::DataDeviceManager *internalDataDeviceManager() {
        return m_internalConnection.ddm;
    }
    KWayland::Client::ShmPool *internalShmPool() {
        return m_internalConnection.shm;
    }
    KWayland::Client::ConnectionThread *internalClientConection() {
        return m_internalConnection.client;
    }
    KWayland::Client::Registry *internalClientRegistry() {
        return m_internalConnection.registry;
    }
    void dispatch();
    quint32 createWindowId(KWayland::Server::SurfaceInterface *surface);

    /**
     * Struct containing information for a created Wayland connection through a
     * socketpair.
     */
    struct SocketPairConnection {
        /**
         * ServerSide Connection
         */
        KWayland::Server::ClientConnection *connection = nullptr;
        /**
         * client-side file descriptor for the socket
         */
        int fd = -1;
    };
    /**
     * Creates a Wayland connection using a socket pair.
     */
    SocketPairConnection createConnection();

    void simulateUserActivity();
    void updateKeyState(KWin::Xkb::LEDs leds);

Q_SIGNALS:
    void shellClientAdded(KWin::XdgShellClient *);
    void shellClientRemoved(KWin::XdgShellClient *);
    void terminatingInternalClientConnection();
    void initialized();
    void foreignTransientChanged(KWayland::Server::SurfaceInterface *child);

private:
    void shellClientShown(Toplevel *t);
    quint16 createClientId(KWayland::Server::ClientConnection *c);
    void destroyInternalConnection();
    template <class T>
    void createSurface(T *surface);
    void initScreenLocker();
    KWayland::Server::Display *m_display = nullptr;
    KWayland::Server::CompositorInterface *m_compositor = nullptr;
    KWayland::Server::SeatInterface *m_seat = nullptr;
    KWayland::Server::DataDeviceManagerInterface *m_dataDeviceManager = nullptr;
    KWayland::Server::XdgShellInterface *m_xdgShell6 = nullptr;
    KWayland::Server::XdgShellInterface *m_xdgShell = nullptr;
    KWayland::Server::PlasmaShellInterface *m_plasmaShell = nullptr;
    KWayland::Server::PlasmaWindowManagementInterface *m_windowManagement = nullptr;
    KWayland::Server::PlasmaVirtualDesktopManagementInterface *m_virtualDesktopManagement = nullptr;
    KWayland::Server::ServerSideDecorationManagerInterface *m_decorationManager = nullptr;
    KWayland::Server::OutputManagementInterface *m_outputManagement = nullptr;
    KWayland::Server::AppMenuManagerInterface *m_appMenuManager = nullptr;
    KWayland::Server::ServerSideDecorationPaletteManagerInterface *m_paletteManager = nullptr;
    KWayland::Server::IdleInterface *m_idle = nullptr;
    KWayland::Server::XdgOutputManagerInterface *m_xdgOutputManager = nullptr;
    KWayland::Server::XdgDecorationManagerInterface *m_xdgDecorationManager = nullptr;
    struct {
        KWayland::Server::ClientConnection *client = nullptr;
        QMetaObject::Connection destroyConnection;
    } m_xwayland;
    KWayland::Server::ClientConnection *m_inputMethodServerConnection = nullptr;
    KWayland::Server::ClientConnection *m_screenLockerClientConnection = nullptr;
    struct {
        KWayland::Server::ClientConnection *server = nullptr;
        KWayland::Client::ConnectionThread *client = nullptr;
        QThread *clientThread = nullptr;
        KWayland::Client::Registry *registry = nullptr;
        KWayland::Client::Compositor *compositor = nullptr;
        KWayland::Client::Seat *seat = nullptr;
        KWayland::Client::DataDeviceManager *ddm = nullptr;
        KWayland::Client::ShmPool *shm = nullptr;
        bool interfacesAnnounced = false;

    } m_internalConnection;
    KWayland::Server::XdgForeignInterface *m_XdgForeign = nullptr;
    KWayland::Server::KeyStateInterface *m_keyState = nullptr;
    QList<XdgShellClient *> m_clients;
    QHash<KWayland::Server::ClientConnection*, quint16> m_clientIds;
    InitalizationFlags m_initFlags;
    QVector<KWayland::Server::PlasmaShellSurfaceInterface*> m_plasmaShellSurfaces;
    KWIN_SINGLETON(WaylandServer)
};

inline
WaylandServer *waylandServer() {
    return WaylandServer::self();
}

} // namespace KWin

#endif

