
#include <osgCEF/CEF>

#include <windows.h>


void CEF::init()
{
    //HINSTANCE h = GetModuleHandle(NULL);
    CefMainArgs args;// (h);
    CefExecuteProcess(args, nullptr, NULL);

    CefSettings settings;
    settings.multi_threaded_message_loop = false;
    settings.windowless_rendering_enabled = true;
    settings.single_process = false;
    settings.command_line_args_disabled = true;
    CefInitialize(args, settings, nullptr, NULL);

}

void CEF::shutdown()
{
    for (int i = 0; i < 100; i++)
        CefDoMessageLoopWork();

    try
    {
        CefShutdown();
    }
    catch (...)
    {
        printf("e");
    }

}


void CEF::update()
{
    CefDoMessageLoopWork();
}

