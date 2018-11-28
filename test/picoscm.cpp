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

using namespace std;
using namespace pscm;

std::wstring s2ws(const std::string& str)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wstr);
}

std::string wc2s(wchar_t wc)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(std::wstring{ wc });
}

int main(int argn, char* argv[])
{
    using pscm::Intern, pscm::Cell, pscm::mknum, pscm::mkstr, pscm::nil;

    //auto oldlocale = std::setlocale(LC_ALL, "en_US.utf8");
    //std::locale::global(std::locale("en_US.utf8"));

    std::string s0 = u8"αβγ€";
    std::wstring w0 = L"€αβγδεζηθιξλμνξοπρστυφχψω";

    wchar_t c0 = w0[3];
    std::string c1 = wc2s(c0);

    //    cout.imbue(std::locale());
    //    cin.imbue(std::locale());
    //    wcout.imbue(std::locale());
    //    wcin.imbue(std::locale());

    //    wcin.sync_with_stdio(false);
    //    wcout.sync_with_stdio(false);

    std::wofstream os("test.txt");
    assert(os);
    os.imbue(std::locale("en_US.utf8"));

    std::ios_base::sync_with_stdio(false);
    wiostream wout{ wcout.rdbuf() };
    wout.copyfmt(wcout);
    wout.clear(wcout.rdstate());
    wout.imbue(std::locale("en_US.utf8"));

    wiostream win{ wcin.rdbuf() };
    win.copyfmt(wcin);
    win.clear(wcin.rdstate());
    win.imbue(std::locale("en_US.utf8"));

    cout << "string: " << ws2s(w0) << " , size: " << w0.size() << endl;
    wout << "wstring: " << w0 << " , size: " << w0.size() << endl;
    wout << "wchar:  " << w0[0] << ' ' << w0[1] << ' ' << w0[24] << endl;

    win >> w0;
    wout << w0 << endl;

    return 0;

    os << w0 << endl;
    os << c0 << endl;
    assert(os.good());
    os.close();

    std::wifstream is("test.txt");
    assert(is);
    is.imbue(std::locale());

    w0.clear();
    c0 = EOF;

    is >> w0;
    is >> c0;
    is.close();

    wcout << w0 << endl;
    wcout << c0 << endl;
    return 0;

    pscm::Scheme scm;

    scm.mkfun("greet", [cntr = 0](Scheme& scm, const SymenvPtr&, const std::vector<Cell>&) mutable -> Cell {
        return scm.list(mkstr("hello world"), mknum(cntr++));
    });

    if (argn > 1)
        scm.load(argv[1]);
    else
        scm.load("picoscmrc.scm");

    scm.repl();
    return 0;

    try {
        pscm::SymenvPtr env = scm.mkenv();

        // (for-each (lambda (x) x) (quote (1 2 3 4)))
        Cell expr = scm.list(Intern::op_foreach,
            scm.list(Intern::_lambda, scm.list(scm.mksym("x")), scm.mksym("x")),
            scm.list(Intern::_quote, scm.list(pscm::mknum(1), pscm::mknum(2))));

        //        pscm::Parser parser(scm);
        //        std::istringstream stream("(define (hello x)  "
        //                                  "   (display x)     "
        //                                  "   (newline))      "
        //                                  "                   "
        //                                  "(define n 43)      "
        //                                  "(hello n)          ");
        //        Cell expr = parser.read(stream);

        Cell proc = scm.eval(env, expr);
        cout << proc << endl;
        return 0;

    } catch (std::bad_variant_access& e) {
        cout << e.what() << endl;

    } catch (std::invalid_argument& e) {
        cout << e.what() << endl;
    }
    return 0;
}
