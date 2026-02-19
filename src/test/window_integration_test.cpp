#include <thread>

#include "detri/window.hpp"

int main()
{
    auto win = detri::window::create("Test Window", 640, 480);
    win.show();
    while (win.is_open())
    {
        while (auto event = win.poll_event())
        {
            if (std::holds_alternative<detri::close_event>(*event))
            {
                win.request_close();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}