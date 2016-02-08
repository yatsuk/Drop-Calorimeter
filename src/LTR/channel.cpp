#include <Winsock2.h>
#include "channel.h"
#include <algorithm>
#include "ltrapi.h"
#include <stdio.h>


/*================================================================================================*/
/*------------------------------------------------------------------------------------------------*/
TChannel::TChannel()
    : socket(INVALID_SOCKET),
      hTimer(0)
{
    default_interval.QuadPart = -10000LL * LTR_DEFAULT_SEND_RECV_TIMEOUT;
    ZeroMemory(&ovRead, sizeof(ovRead));
    ZeroMemory(&ovWrite, sizeof(ovWrite));
    ovRead.hEvent = ovWrite.hEvent = WSA_INVALID_EVENT;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), & wsaData);
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
void TChannel::Close()
{
    if (socket != INVALID_SOCKET)
    {
        shutdown(socket, SD_BOTH);
        closesocket(socket);
        socket = INVALID_SOCKET;
    }
    if (ovWrite.hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(ovWrite.hEvent);
        ovWrite.hEvent = WSA_INVALID_EVENT;
    }
    if (ovRead.hEvent != WSA_INVALID_EVENT)
    {
        WSACloseEvent(ovRead.hEvent);
        ovRead.hEvent = WSA_INVALID_EVENT;
    }
    if (hTimer)
    {
        CloseHandle(hTimer);
        hTimer = 0;
    }
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
BOOL TChannel::Open(DWORD addr, WORD port, DWORD timeout_ms)
{
    Close();
    SOCKET s = INVALID_SOCKET;
    s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET)
        return FALSE;

    // Создаем таймер для таймаутов
    hTimer = CreateWaitableTimer(NULL, TRUE, NULL);

    // Создаем ссобытия
    ovRead.hEvent = WSACreateEvent();
    ovWrite.hEvent = WSACreateEvent();

    if ((ovRead.hEvent == WSA_INVALID_EVENT) || (ovWrite.hEvent == WSA_INVALID_EVENT) || !hTimer)
    {
        Close();
        return FALSE;
    }

    sockaddr_in peer;
    ZeroMemory(&peer, sizeof(peer));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = htonl(addr);

    // Соединение выполняется в обычном режиме (blocking socket),
    // поэтому таймаут может быть намного больше заданного.
    // Значение timeout_ms используется для того, чтобы повторять попытки соединения
    // при нефатальных ошибках (например, ltrserver еще не загрузился).
    int res;
    SetTimer(timeout_ms);
    do
    {
        res = WSAConnect(s, (sockaddr*)&peer, sizeof(peer), NULL, NULL, NULL, NULL);
        if (SOCKET_ERROR == res)
        {
            DWORD err = WSAGetLastError();
            // Если ошибка фатальная, прервать цикл
            if ((WSAECONNREFUSED != err) && (WSAENETUNREACH != err) &&
                    (WSAEHOSTUNREACH != err) && (WSAETIMEDOUT != err))
                break;
            Sleep(50);
        }
    }
    while ((SOCKET_ERROR == res) && !TimerExpired());

    if (SOCKET_ERROR == res)
    {
        Close();
        return FALSE;
    }
    else
    {
        socket = s;
        return TRUE;
    }
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT TChannel::Recv(LPBYTE buf, DWORD size)
{
    HANDLE evt[2];
    if (!buf) return LTR_ERROR_PARAMETERS;
    if (!IsOpen())
    {
        Sleep(1);
        return LTR_ERROR_CHANNEL_CLOSED;
    }

    WSABUF wb;
    DWORD offset = 0;
    DWORD xfer_size, flag;
    bool timed_out = false;
    bool error = false;

    evt[0] = ovRead.hEvent;
    evt[1] = hTimer;
    while (!error && !timed_out && (offset < size))
    {
        wb.buf = reinterpret_cast<char *>(buf + offset);
        wb.len = size - offset;
        flag = 0;
        xfer_size = 0;
        if (SOCKET_ERROR == WSARecv(socket, &wb, 1, &xfer_size, &flag, &ovRead, NULL))
        {
            error = (WSA_IO_PENDING != WSAGetLastError());
            if (!error)
            {
                switch (WSAWaitForMultipleEvents(2, evt, FALSE, INFINITE, FALSE))
                {
                case WSA_WAIT_EVENT_0:
                    error = !WSAGetOverlappedResult(socket, &ovRead, &xfer_size, FALSE, &flag);
                    // если соединение закрыто внешним хостом, то выходим как по таймауту
                    if (!error && (xfer_size == 0))
                        timed_out = true;
                    break;
                case WSA_WAIT_EVENT_0 + 1: // таймаут - не ошибка
                    timed_out = true;
                    CancelIo((HANDLE)socket);
                    WSAGetOverlappedResult(socket, &ovRead, &xfer_size, TRUE, &flag);
                    break;
                default:
                    error = true;
                    CancelIo((HANDLE)socket);
                    WSAGetOverlappedResult(socket, &ovRead, &xfer_size, TRUE, &flag);
                    break;
                }
            }
        }
        offset += xfer_size;
    }

    return (error) ? LTR_ERROR_RECV : offset;
}
/*------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------*/
INT TChannel::Send(LPBYTE buf, DWORD size)
{
    HANDLE evt[2];
    if (!buf) return LTR_ERROR_PARAMETERS;
    if (!IsOpen())
    {
        Sleep(1);
        return LTR_ERROR_CHANNEL_CLOSED;
    }

    if (!size) return 0;

    WSABUF wb;
    DWORD xfer_size, flag;
    bool error = false;

    evt[0] = ovWrite.hEvent;
    evt[1] = hTimer;
    wb.buf = reinterpret_cast<char *>(buf);
    wb.len = size;
    xfer_size = 0;
    if (SOCKET_ERROR == WSASend(socket, &wb, 1, &xfer_size, 0, &ovWrite, NULL))
    {
        error = (WSA_IO_PENDING != WSAGetLastError());
        if (!error)
        {
            switch (WSAWaitForMultipleEvents(2, evt, FALSE, INFINITE, FALSE))
            {
            case WSA_WAIT_EVENT_0:
                error = !WSAGetOverlappedResult(socket, &ovWrite, &xfer_size, FALSE, &flag);
                break;
            case WSA_WAIT_EVENT_0 + 1: // таймаут - не ошибка
                CancelIo((HANDLE)socket);
                WSAGetOverlappedResult(socket, &ovWrite, &xfer_size, TRUE, &flag);
                break;
            default:
                error = true;
                CancelIo((HANDLE)socket);
                WSAGetOverlappedResult(socket, &ovWrite, &xfer_size, TRUE, &flag);
                break;
            }
        }
    }

    return (error) ? LTR_ERROR_SEND : xfer_size;
}
/*------------------------------------------------------------------------------------------------*/
