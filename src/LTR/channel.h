#ifndef __CHANNEL__
#define __CHANNEL__
#include <Winsock2.h>

/*================================================================================================*/
class TChannel
    {
private:
    SOCKET socket;
    HANDLE hTimer;
    WSAOVERLAPPED ovRead;
    WSAOVERLAPPED ovWrite;
    LARGE_INTEGER default_interval;

    LONGLONG convert_timer(DWORD timeout_ms)
        {
        return (timeout_ms != INFINITE) ? (-10000LL * timeout_ms) : (1LL << 63);
        }
public:
    void SetDefaultTimeout(DWORD timeout_ms)
        {
        default_interval.QuadPart = convert_timer(timeout_ms);
        }

    BOOL Open(DWORD addr, WORD port, DWORD timeout_ms);

    void Close();

    BOOL IsOpen() const
        {
        return (socket != INVALID_SOCKET);
        }

    void SetTimer(DWORD timeout_ms)
        {
        LARGE_INTEGER interval;
        interval.QuadPart = convert_timer(timeout_ms);
        SetWaitableTimer(hTimer, &interval, 0, NULL, NULL, FALSE);
        }

    void SetTimerDefault()
        {
        SetWaitableTimer(hTimer, &default_interval, 0, NULL, NULL, FALSE);
        }

    BOOL TimerExpired(DWORD wait_ms = 0) const
        {
        return (hTimer && (WaitForSingleObject(hTimer, wait_ms) == WAIT_OBJECT_0));
        }

    // При вызове Recv() и Send() таймер должен быть заранее запущен через SetTimer()
    INT Recv(LPBYTE buf, DWORD size);
    INT Send(LPBYTE buf, DWORD size);

    TChannel();
    ~TChannel()
        {
        Close();
        }
    };
/*================================================================================================*/
#endif
