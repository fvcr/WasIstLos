#include <clocale>
#include <iostream>
#include <glibmm/i18n.h>
#include "Config.hpp"
#include "ui/Application.hpp"
#include "ui/MainWindow.hpp"
#include "util/Helper.hpp"
#include "util/Settings.hpp"

namespace
{
    void sigterm(int)
    {
        wfl::ui::Application::getInstance().quit();
    }
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    bindtextdomain(GETTEXT_PACKAGE, WFL_LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    auto app = wfl::ui::Application{argc, argv, WFL_APP_ID, Gio::APPLICATION_HANDLES_OPEN};

    wfl::util::redirectOutputToLogger();

    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
    signal(SIGPIPE, SIG_IGN);

    auto mainWindow = std::unique_ptr<wfl::ui::MainWindow>{};

    try
    {
        auto const refBuilder = Gtk::Builder::create_from_resource("/main/ui/MainWindow.ui");

        wfl::ui::MainWindow* window = nullptr;
        refBuilder->get_widget_derived("window_main", window);
        mainWindow.reset(window);
    }
    catch (Glib::Exception const& error)
    {
        std::cerr << "Failed to load ui resource: " << error.what().c_str() << std::endl;
        return 1;
    }

    app.signal_open().connect(
        [&app, &mainWindow](Gtk::Application::type_vec_files const& files, const Glib::ustring&)
        {
            if (!files.empty())
            {
                // Activate the application if it's not running
                app.activate();
                mainWindow->openUrl(files.at(0U)->get_uri());
            }
        });

    if (wfl::util::Settings::getInstance().getValue<bool>("general", "start-in-tray")
        && wfl::util::Settings::getInstance().getValue<bool>("general", "close-to-tray"))
    {
        mainWindow->hide();
        wfl::ui::Application::getInstance().keepAlive();
        return app.run();
    }
    else if (wfl::util::Settings::getInstance().getValue<bool>("general", "start-minimized"))
    {
        mainWindow->iconify();
    }

    return app.run(*mainWindow);
}
