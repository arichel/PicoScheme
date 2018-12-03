/********************************************************************************/ /**
 * @file pico.cpp
 *
 *************************************************************************************/
#include <assert.h>
#include <iostream>
#include <memory>

#include <codecvt>
#include <locale>

#include <picoscm/scheme.hpp>
#include <picoscm/utils.hpp>

using namespace std;
using namespace pscm;

//std::string ws2s(const std::wstring& wstr)
//{
//    using convert_type = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_type, wchar_t> converter;
//    return converter.to_bytes(wstr);
//}

//std::string wc2s(wchar_t wc)
//{
//    using convert_type = std::codecvt_utf8<wchar_t>;
//    std::wstring_convert<convert_type, wchar_t> converter;
//    return converter.to_bytes(std::wstring{ wc });
//}

/**
 * Enable locale globally and set all standard io-ports accordingly.
 * @param name Name of the locale.
 */
void enable_locale(const char* name = "en_US.utf8")
{
    std::ios_base::sync_with_stdio(false);

    std::setlocale(LC_ALL, name);
    std::locale loc{ name };
    std::locale::global(loc);

    cout.imbue(loc);
    cerr.imbue(loc);
    clog.imbue(loc);
    wcout.imbue(loc);
    wcerr.imbue(loc);
    wclog.imbue(loc);
    cin.imbue(loc);
    wcin.imbue(loc);
}

int main(int argn, char* argv[])
{
    using pscm::Intern, pscm::Cell, pscm::mknum, pscm::str, pscm::nil;

    enable_locale();
    pscm::Scheme scm;

    pscm::fun(scm, "greet", [cntr = 0](Scheme& scm, const SymenvPtr&, const std::vector<Cell>&) mutable -> Cell {
        return scm.list(pscm::str("hello world"), mknum(cntr++));
    });

    //    using T = decltype(argv[0]);

    //    TDEF<pscm::char_traits<T>::char_type> t0;

    //using C = typename pscm::char_traits<std::decay_t<T>>::char_type;
    //TDEF<C> t;

    if (argn > 1)
        load(scm, argv[0]);
    else
        load(scm, "picoscmrc.scm"s);

    scm.repl();
    return 0;

    //    try {
    //        pscm::SymenvPtr env = scm.mkenv();

    //        // (for-each (lambda (x) x) (quote (1 2 3 4)))
    //        Cell expr = scm.list(Intern::op_foreach,
    //            scm.list(Intern::_lambda, scm.list(scm.mksym(L"x")), scm.mksym(L"x")),
    //            scm.list(Intern::_quote, scm.list(pscm::mknum(1), pscm::mknum(2))));

    //        //        pscm::Parser parser(scm);
    //        //        std::istringstream stream("(define (hello x)  "
    //        //                                  "   (display x)     "
    //        //                                  "   (newline))      "
    //        //                                  "                   "
    //        //                                  "(define n 43)      "
    //        //                                  "(hello n)          ");
    //        //        Cell expr = parser.read(stream);

    //        Cell proc = scm.eval(env, expr);
    //        wcout << proc << endl;
    //        return 0;

    //    } catch (std::bad_variant_access& e) {
    //        cout << e.what() << endl;

    //    } catch (std::invalid_argument& e) {
    //        cout << e.what() << endl;
    //    }
    //    return 0;
}
