#include "Application.hpp"
#include <gtkmm/window.h>
#include <gtkmm/settings.h>
#include "Config.hpp"
#include "../util/Settings.hpp"

namespace wfl::ui
{
    Application* Application::m_instance = nullptr;

    Application& Application::getInstance()
    {
        return *m_instance;
    }

    Application::Application(int argc, char** argv)
        : Gtk::Application{argc, argv, WFL_APP_ID, Gio::APPLICATION_HANDLES_OPEN}
        , m_onHold{false}
        , m_mainWindow{nullptr}
    {
        if (!m_instance)
        {
            m_instance = this;
        }
        else
        {
            throw std::runtime_error("Attempt to create more than one instance of the Application");
        }
    }

    void Application::keepAlive()
    {
        if (!m_onHold)
        {
            hold();
            m_onHold = true;
        }
    }

    void Application::endKeepAlive()
    {
        if (m_onHold)
        {
            release();
            m_onHold = false;
        }
    }

    void Application::on_startup()
    {
        Gtk::Application::on_startup();

        auto const refBuilder = Gtk::Builder::create_from_resource("/main/ui/MainWindow.ui");

        wfl::ui::MainWindow* window = nullptr;
        refBuilder->get_widget_derived("window_main", window);
        m_mainWindow.reset(window);

        add_window(*m_mainWindow);
    }

    void Application::on_activate()
    {
        Gtk::Application::on_activate();

        if (wfl::util::Settings::getInstance().getValue<bool>("general", "start-in-tray")
            && wfl::util::Settings::getInstance().getValue<bool>("general", "close-to-tray"))
        {
            if (!m_onHold)
            {
                keepAlive();
                m_mainWindow->hide();
            }
            else
            {
                m_mainWindow->present();
            }
        }
        else if (wfl::util::Settings::getInstance().getValue<bool>("general", "start-minimized"))
        {
            m_mainWindow->is_visible() ? m_mainWindow->iconify() : m_mainWindow->deiconify();
        }

        auto const settings   = Gtk::Settings::get_default();
        auto const preferDark = util::Settings::getInstance().getValue<bool>("appearance", "prefer-dark-theme");
        settings->property_gtk_application_prefer_dark_theme().set_value(preferDark);
    }

    void Application::on_open(Application::type_vec_files const& files, Glib::ustring const&)
    {
        if (!files.empty())
        {
            activate();

            if (m_mainWindow)
            {
                m_mainWindow->openUrl(files.at(0U)->get_uri());
            }
        }
    }
}
