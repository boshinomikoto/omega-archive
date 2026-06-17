#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include <string>
#include <vector>
#include <sstream>
#include "mylib/omegadb.h"
#include "mylib/auth.h"

// splits a multiline string
ftxui::Element multiline_text(const std::string& str)
{
    std::vector<ftxui::Element> lines;
    std::istringstream iss(str);
    std::string line;
    while (std::getline(iss, line))
        lines.push_back(ftxui::text(line));
    return ftxui::vbox(std::move(lines));
}

// maps menu index to the right DB query
std::string selectOp(sqlite3 *db, int option)
{
    switch (option)
    {
        case 0: return db_select_all(db);
        case 1: return db_select_by_degree(db, "bachelor");
        case 2: return db_select_by_course(db, "bachelors_1course");
        case 3: return db_select_by_course(db, "bachelors_2course");
        case 4: return db_select_by_course(db, "bachelors_3course");
        case 5: return db_select_by_course(db, "bachelors_4course");
        case 6: return db_select_by_degree(db, "master");
        case 7: return db_select_by_course(db, "masters_course1");
        case 8: return db_select_by_course(db, "masters_course2");
        case 9: return db_select_by_degree(db, "graduates");
        default: return "";
    }
}

// ── auth screen

bool run_auth(sqlite3 *auth_db)
{
    std::vector<std::string> tabs = {"  Login  ", "  Register  "};
    int tab = 0;

    std::string log1, pass1;
    std::string log2, pass2;
    std::string msg;
    bool success = false;

    ftxui::InputOption pass_opt;
    pass_opt.password = true;

    auto in_log1  = ftxui::Input(&log1,  "login");
    auto in_pass1 = ftxui::Input(&pass1, "password", pass_opt);
    auto in_log2  = ftxui::Input(&log2,  "login");
    auto in_pass2 = ftxui::Input(&pass2, "password", pass_opt);

    auto auth_screen = ftxui::ScreenInteractive::TerminalOutput();

    auto btn_login = ftxui::Button("  Login  ", [&]
    {
        msg.clear();
        if (log1.empty() || pass1.empty()) { msg = "Fill in all fields"; return; }
        int r = auth_login(auth_db, log1.c_str(), pass1.c_str());
        if (r == 1) { success = true; auth_screen.Exit(); }
        else        { msg = "Wrong login or password"; }
    });

    auto btn_reg = ftxui::Button("  Register  ", [&]
    {
        msg.clear();
        if (log2.empty() || pass2.empty()) { msg = "fill in all fields"; return; }
        int r = auth_register(auth_db, log2.c_str(), pass2.c_str());
        if (r == SQLITE_OK) { msg = "done! Now log in."; tab = 0; log1 = log2; pass1.clear(); }
        else                { msg = "login already taken"; }
    });

    auto toggle = ftxui::Toggle(&tabs, &tab);

    auto login_cont = ftxui::Container::Vertical({in_log1, in_pass1, btn_login});
    auto reg_cont   = ftxui::Container::Vertical({in_log2, in_pass2, btn_reg});
    auto tab_body   = ftxui::Container::Tab({login_cont, reg_cont}, &tab);
    auto auth_cont  = ftxui::Container::Vertical({toggle, tab_body});

    auto renderer = ftxui::Renderer(auth_cont, [&]
    {
        ftxui::Element body;
        if (tab == 0)
        {
            body = ftxui::vbox({
                ftxui::hbox(ftxui::text("Login:    "), in_log1->Render()),
                ftxui::hbox(ftxui::text("Password: "), in_pass1->Render()),
                ftxui::separator(),
                btn_login->Render() | ftxui::center,
            });
        }
        else
        {
            body = ftxui::vbox({
                ftxui::hbox(ftxui::text("Login:    "), in_log2->Render()),
                ftxui::hbox(ftxui::text("Password: "), in_pass2->Render()),
                ftxui::separator(),
                btn_reg->Render() | ftxui::center,
            });
        }

        bool is_ok = !msg.empty() && msg.find("Done") == 0;

        return ftxui::vbox({
            ftxui::text("OmegaDB") | ftxui::bold | ftxui::center,
            ftxui::separator(),
            toggle->Render() | ftxui::center,
            ftxui::separator(),
            body,
            ftxui::separator(),
            msg.empty()
                ? ftxui::text(" ")
                : ftxui::text(msg) | ftxui::color(is_ok ? ftxui::Color::Green : ftxui::Color::Red) | ftxui::center,
        }) | ftxui::border | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36);
    });

    auto component = ftxui::CatchEvent(renderer, [&](ftxui::Event e) {
        if (e == ftxui::Event::Escape) { auth_screen.Exit(); return true; }
        return false;
    });

    auth_screen.Loop(component);
    return success;
}

// ── main app

int main()
{
    // auth db
    sqlite3 *auth_db = nullptr;
    if (auth_db_connect(&auth_db, "users.db") != SQLITE_OK)
    {
        std::cerr << "Auth DB connection failed\n";
        return 1;
    }
    auth_db_init(auth_db);

    if (!run_auth(auth_db))
    {
        auth_db_close(auth_db);
        return 0; // user pressed Esc without logging in
    }
    auth_db_close(auth_db);

    // main db
    sqlite3 *db = nullptr;
    if (db_connect(&db, "omega.db") != SQLITE_OK)
    {
        std::cerr << "connection failed: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }
    db_init_table(db);
    db_fill_test_data(db);

    std::vector<std::string> menuItems = {
        "all students",
        "bachelors",
        "bachelors 1course", "bachelors 2course", "bachelors 3course", "bachelors 4course",
        "masters",
        "masters course1", "masters course2",
        "graduates"
    };
    int selectedOption = 0;

    std::string firstName, secondName, group, result;

    auto input_name    = ftxui::Input(&firstName,  "");
    auto input_surname = ftxui::Input(&secondName, "");
    auto input_group   = ftxui::Input(&group,      "");

    auto menu_opt = ftxui::MenuOption::Vertical();
    menu_opt.on_change = [&]{ result.clear(); }; // clear search result when menu selection changes
    auto main_menu = ftxui::Menu(&menuItems, &selectedOption, menu_opt);

    auto btn_submit = ftxui::Button("  Search  ", [&]
    {
        if (!firstName.empty() && !secondName.empty())
            result = db_select_by_name_and_surname(db, firstName.c_str(), secondName.c_str());
        else if (!firstName.empty())  result = db_select_by_name(db, firstName.c_str());
        else if (!secondName.empty()) result = db_select_by_surname(db, secondName.c_str());
        else if (!group.empty())      result = db_select_by_group(db, group.c_str());
        else                          result.clear();

        // clear the search fields after the query
        firstName.clear();
        secondName.clear();
        group.clear();
    });

    auto container = ftxui::Container::Vertical({
        main_menu, input_name, input_surname, input_group, btn_submit,
    });

    auto renderer = ftxui::Renderer(container, [&]
    {
        auto sidebar = ftxui::vbox({
            ftxui::text("Menu") | ftxui::bold | ftxui::center,
            ftxui::separator(),
            main_menu->Render() | ftxui::flex,
            ftxui::separator(),
            ftxui::vbox(ftxui::text("Name:"),    input_name->Render()),
            ftxui::vbox(ftxui::text("Surname:"), input_surname->Render()),
            ftxui::vbox(ftxui::text("Group:"),   input_group->Render()),
            ftxui::separator(),
            btn_submit->Render() | ftxui::center,
        }) | ftxui::border | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 25);

        auto content = ftxui::vbox({
            ftxui::text("Panel") | ftxui::bold | ftxui::center,
            ftxui::separator(),
            // if we have a search result show it, otherwise show menu selection
            multiline_text(!result.empty() ? result : selectOp(db, selectedOption))
                | ftxui::color(ftxui::Color::Green),
            ftxui::filler(),
            ftxui::text("Esc - exit") | ftxui::dim,
        }) | ftxui::border | ftxui::flex;

        return ftxui::hbox({sidebar, content})
            | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 30);
    });

    auto screen = ftxui::ScreenInteractive::TerminalOutput();
    auto component = ftxui::CatchEvent(renderer, [&](ftxui::Event e) {
        if (e == ftxui::Event::Escape) { screen.Exit(); return true; }
        return false;
    });
    screen.Loop(component);

    db_close(db);
    return 0;
}
