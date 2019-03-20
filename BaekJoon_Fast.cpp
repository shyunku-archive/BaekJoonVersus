#include <iostream>
#include <string>
#include <vector>
#include <urlmon.h>
#include <fstream>
#include <Windows.h>
#include <WinInet.h>
#include <locale>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

#define USER_INFO_TEMPLATE                              "https://www.acmicpc.net/user/"
#define ORGANIZATION_INFO_TEMPLATE              "https://www.acmicpc.net/school/ranklist/"
#define COMPETETION_INFO_TEMPLATE               "https://www.acmicpc.net/vs/"
#define USER_INFO_DOWNLOAD_TEMPLATE             "USERINFO-"
#define ORGANIZATION_DOWNLOAD_TEMPLATE  "ORGZINFO-"
#define COMPETETION_DOWNLOAD_TEMPLATE   "COMPINFO-"
#define COMPETETION_FILENAME_TAG                "vs"
#define HTML_EXTENSION                                  ".html"
#define STANDARD_SLASH                                  "/"
#define SBLANK                                                  ""

#define RANKING_KEYWORD                                 "<th>랭킹</th>"
#define SOLVED_KEYWORD                                  "<th>푼문제</th>"
#define SUBMISSION_KEYWORD                              "<th>제출</th>"
#define CORRECTED_KEYWORD                               "<th>맞았습니다</th>"
#define WRONG_KEYWORD                                   "<th>틀렸습니다</th>"
#define COMMON_KEYWORD1                                 "</a>모두푼문제-"
#define COMMON_KEYWORD2                                 "문제</h3>"
#define ORGANI_KEYWORD1                                 "<title>"
#define ORGANI_KEYWORD2                                 "랭킹"
#define ORGANI_NAMETAG1                                 "<th>학교/회사</th><td>"
#define ORGANI_NAMETAG2                                 "</td></tr>"


using namespace std;

int TOTAL_STUDENT = 119427;
string USER_NAME, OPPS_NAME;
class information {
public:
        string username;
        string oppsname;
        int ranking;
        int solved_problem_num;
        int submission;
        int corrected;
        int wrong;
        int common_solved_problem_num;
        int subranking;
        string ORGANIZATION_CODE;
        string organization;
        double correct_rate;
        information() {}
};

information user, opps;

string substring(string str, size_t start, size_t end) {
        return str.substr(start, end - start + 1);
}

void INITIAL_SETTING_FOR_OPTIMIZING() {
        cin.tie(NULL);
        ios::sync_with_stdio(false);
        /* Don't use scanf, printf, puts, getchar, putchar in submission */
}

std::wstring string_to_wstring(const std::string& s)
{
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
}

wstring URL_combinate_to_LPCWSTR(string str1, string str2, string str3, string str4, bool isHTML) {
        string orgURL = str1 + str2 + str3 + str4 + (isHTML ? HTML_EXTENSION : "");
        if (isHTML) cout << "Downloading file as ";
        cout << orgURL;
        if (isHTML) cout << " from "; else cout << endl;
        return string_to_wstring(orgURL);
}

void download_detail(wstring url, wstring filename) {
        BOOL ERRNO = URLDownloadToFile(NULL, url.c_str(), filename.c_str(), 0, NULL);
        if (ERRNO != S_OK) {
                cout << "ERROR! FILE DOWNLOAD FAILED!" << endl;
                exit(0);
        }
        //Delete Cashfile
        cout << "FILE DOWNLOAD SUCCESS!"  << endl;
        if (SUCCEEDED(ERRNO))
                DeleteUrlCacheEntry(url.c_str());
}

void downloadAll() {
        bool ERRNO;
        //Individual Information
        download_detail(
                URL_combinate_to_LPCWSTR(USER_INFO_TEMPLATE, USER_NAME, SBLANK, SBLANK, false),
                URL_combinate_to_LPCWSTR(USER_INFO_DOWNLOAD_TEMPLATE, USER_NAME, SBLANK, SBLANK, true)
        );
        //Opposite Information
        download_detail(
                URL_combinate_to_LPCWSTR(USER_INFO_TEMPLATE, OPPS_NAME, SBLANK, SBLANK, false),
                URL_combinate_to_LPCWSTR(USER_INFO_DOWNLOAD_TEMPLATE, OPPS_NAME, SBLANK, SBLANK, true)
        );

        //Competition Information
        download_detail(
                URL_combinate_to_LPCWSTR(COMPETETION_INFO_TEMPLATE, USER_NAME, STANDARD_SLASH, OPPS_NAME, false),
                URL_combinate_to_LPCWSTR(COMPETETION_DOWNLOAD_TEMPLATE, USER_NAME, COMPETETION_FILENAME_TAG, OPPS_NAME, true)
        );
}

char* UTF8ToANSI(const char *pszCode) {
        BSTR    bstrWide;
        char*   pszAnsi;
        int     nLength;

        nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen((LPCWSTR)pszCode) + 1, NULL, NULL);
        bstrWide = SysAllocStringLen(NULL, nLength);

        MultiByteToWideChar(CP_UTF8, 0, pszCode, lstrlen((LPCWSTR)pszCode) + 1, bstrWide, nLength);

        nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
        pszAnsi = new char[nLength];

        WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
        SysFreeString(bstrWide);

        return pszAnsi;
}

string getSpecificString(string original, string keyword, string tag) {
        size_t found = original.find(keyword);
        if (found == string::npos) {
                cout << "키워드 [" << keyword << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        string start_tag = "<" + tag + ">", end_tag = "</" + tag + ">";
        size_t start_flag = original.find(start_tag, found + 1) + start_tag.length();
        size_t end_flag = original.find(end_tag, found + 1) - 1;

        string extracted = substring(original, start_flag, end_flag);
        int bracket = 0, flag = 0;
        //cout << "Extracted : " << extracted << endl << endl;
        for (size_t i = 0; i < extracted.length(); i++)
                if (extracted[i] == '<')
                        bracket++;
        if (bracket == 0)return extracted;
        for (size_t i = 0; i < extracted.length(); i++) {
                if (extracted[i] == '>')
                        flag++;
                if (flag == bracket / 2) {
                        start_flag = i + 1;
                        break;
                }
        }
        for (size_t i = start_flag; i < extracted.length(); i++) {
                if (extracted[i] == '<') {
                        end_flag = i - 1;
                        break;
                }
        }
        extracted = substring(extracted, start_flag, end_flag);
        return extracted;
}

string getSpecificStringCloseDetail(string original, string keyword, string keyword2) {
        size_t found = original.find(keyword);
        if (found == string::npos) {
                cout << "키워드 [" << keyword << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        size_t start_flag = found + keyword.length();
        found = original.find(keyword2, found + 1);
        if (found == string::npos) {
                cout << "키워드 [" << keyword2 << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        size_t end_flag = found - 1;
        string extracted = substring(original, start_flag, end_flag);
        return extracted;
}

string getSpecificStringCloseDetail2(string original, string keyword, int bracket) {
        size_t found = original.find(keyword);
        if (found == string::npos) {
                cout << "키워드 [" << keyword << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        found = original.find(keyword,found+1);
        if (found == string::npos) {
                cout << "키워드 [" << keyword << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        found = original.find(keyword, found + 1);
        if (found == string::npos) {
                cout << "키워드 [" << keyword << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        found = original.find(keyword, found + 1);
        if (found == string::npos) {
                cout << "키워드 [" << keyword << "] 를 찾지 못했습니다. 사이트 HTML Formation을 재확인 해주세요." << endl;
                exit(0);
        }
        size_t start_flag = found + 1;
        int temp_cnt = 0;
        for (int i = found+1; i < original.length(); i++) {
                if (original[i] == '>')
                        temp_cnt++;
                if (temp_cnt == bracket) {
                        start_flag = i + 1;
                        break;
                }
        }
        size_t end_flag = found + 1;
        for (int i = found + 1; i < original.length(); i++) {
                if (original[i] == '<'){
                        end_flag = i - 1;
                        break;
                }
        }
        string extracted = substring(original, start_flag, end_flag);
        return extracted;
}

void parseUserInfo() {
        string filename = USER_INFO_DOWNLOAD_TEMPLATE; filename += USER_NAME; filename += HTML_EXTENSION;
        ifstream iistream(filename);
        string str = "";
        char c;
        char collector;
        int iterator = 0, zero = 0;
        while (!iistream.eof()) {
                iistream >> c;
                str += c;
        }
        string WholeString(UTF8ToANSI(str.c_str()));

        iistream.close();
        cout << filename << " :: " << WholeString.length()/1000 << " KB converted." << endl;
        //Converting Complete.
        //Ranking
        user.ranking = atoi(getSpecificString(WholeString, RANKING_KEYWORD, "td").c_str());
        //Solved Problem number
        user.solved_problem_num = atoi(getSpecificString(WholeString, SOLVED_KEYWORD, "td").c_str());
        //Submission
        user.submission = atoi(getSpecificString(WholeString, SUBMISSION_KEYWORD, "td").c_str());
        //Corrected
        user.corrected = atoi(getSpecificString(WholeString, CORRECTED_KEYWORD, "td").c_str());
        //Wrong
        user.wrong = atoi(getSpecificString(WholeString, WRONG_KEYWORD, "td").c_str());

        user.username = USER_NAME;
        user.oppsname = OPPS_NAME;
        string O_ = getSpecificStringCloseDetail(WholeString, ORGANI_NAMETAG1, ORGANI_NAMETAG2);
        bool islink = false;
        for(int i=0;i<O_.length();i++)
                if (O_[i] == '<') {
                        O_ = getSpecificStringCloseDetail(O_, "ranklist/", "\">");
                        user.ORGANIZATION_CODE = O_;
                        islink = true;
                        break;
                }
        if (!islink) user.ORGANIZATION_CODE = "";
}

void parseOppsInfo() {
        string filename = USER_INFO_DOWNLOAD_TEMPLATE; filename += OPPS_NAME; filename += HTML_EXTENSION;
        ifstream iistream(filename);
        string str = "";
        char c;
        char collector;
        int iterator = 0, zero = 0;
        while (!iistream.eof()) {
                iistream >> c;
                str += c;
        }
        string WholeString(UTF8ToANSI(str.c_str()));

        iistream.close();
        cout << filename << " :: " << WholeString.length() / 1000 << " KB converted." << endl;
        //Converting Complete.
        //Ranking
        opps.ranking = atoi(getSpecificString(WholeString, RANKING_KEYWORD, "td").c_str());
        //Solved Problem number
        opps.solved_problem_num = atoi(getSpecificString(WholeString, SOLVED_KEYWORD, "td").c_str());
        //Submission
        opps.submission = atoi(getSpecificString(WholeString, SUBMISSION_KEYWORD, "td").c_str());
        //Corrected
        opps.corrected = atoi(getSpecificString(WholeString, CORRECTED_KEYWORD, "td").c_str());
        //Wrong
        opps.wrong = atoi(getSpecificString(WholeString, WRONG_KEYWORD, "td").c_str());

        opps.username = OPPS_NAME;
        opps.oppsname = USER_NAME;

        string O_ = getSpecificStringCloseDetail(WholeString, ORGANI_NAMETAG1, ORGANI_NAMETAG2);
        bool islink = false;
        for (int i = 0; i<O_.length(); i++)
                if (O_[i] == '<') {
                        O_ = getSpecificStringCloseDetail(O_, "ranklist/", "\">");
                        opps.ORGANIZATION_CODE = O_;
                        islink = true;
                        break;
                }
        if (!islink) opps.ORGANIZATION_CODE = "";
}

void parseCompeInfo() {
        string filename = COMPETETION_DOWNLOAD_TEMPLATE; filename += USER_NAME; filename += COMPETETION_FILENAME_TAG; filename += OPPS_NAME; filename += HTML_EXTENSION;
        ifstream iistream(filename);
        string str = "";
        char c;
        char collector;
        int iterator = 0, zero = 0;
        while (!iistream.eof()) {
                iistream >> c;
                str += c;
        }
        string WholeString(UTF8ToANSI(str.c_str()));

        iistream.close();
        cout << filename << " :: " << WholeString.length() / 1000 << " KB converted." << endl;
        //Converting Complete.
        //Common Solved Problem
        opps.common_solved_problem_num = user.common_solved_problem_num = atoi(getSpecificStringCloseDetail(WholeString, COMMON_KEYWORD1, COMMON_KEYWORD2).c_str());
}

void parseUserOrganInfo() {
        if (user.ORGANIZATION_CODE != "") {
                download_detail(
                        URL_combinate_to_LPCWSTR(ORGANIZATION_INFO_TEMPLATE, user.ORGANIZATION_CODE, SBLANK, SBLANK, false),
                        URL_combinate_to_LPCWSTR(ORGANIZATION_DOWNLOAD_TEMPLATE, USER_NAME, SBLANK, SBLANK, true)
                );
        }
        else return;
        string filename = ORGANIZATION_DOWNLOAD_TEMPLATE; filename += USER_NAME; filename += HTML_EXTENSION;
        ifstream iistream(filename);
        string str = "";
        char c;
        char collector;
        int iterator = 0, zero = 0;
        while (!iistream.eof()) {
                iistream >> c;
                str += c;
        }
        string WholeString(UTF8ToANSI(str.c_str()));

        iistream.close();
        cout << filename << " :: " << WholeString.length() / 1000 << " KB converted." << endl;
        //Converting Complete.
        //SubRanking
        user.subranking = atoi(getSpecificStringCloseDetail2(WholeString, USER_NAME, 8).c_str()) - 1;
        user.correct_rate = stod(getSpecificStringCloseDetail2(WholeString, USER_NAME, 4).c_str());
        user.organization = getSpecificStringCloseDetail(WholeString, ORGANI_KEYWORD1, ORGANI_KEYWORD2);
}

void parseOppsOrganInfo() {
        if (opps.ORGANIZATION_CODE != "") {
                download_detail(
                        URL_combinate_to_LPCWSTR(ORGANIZATION_INFO_TEMPLATE, opps.ORGANIZATION_CODE, SBLANK, SBLANK, false),
                        URL_combinate_to_LPCWSTR(ORGANIZATION_DOWNLOAD_TEMPLATE, OPPS_NAME, SBLANK, SBLANK, true)
                );
        }
        else return;
        string filename = ORGANIZATION_DOWNLOAD_TEMPLATE; filename += OPPS_NAME; filename += HTML_EXTENSION;
        ifstream iistream(filename);
        string str = "";
        char c;
        char collector;
        int iterator = 0, zero = 0;
        while (!iistream.eof()) {
                iistream >> c;
                str += c;
        }
        string WholeString(UTF8ToANSI(str.c_str()));

        iistream.close();
        cout << filename << " :: " << WholeString.length() / 1000 << " KB converted." << endl;
        //Converting Complete.
        //SubRanking
        opps.subranking = atoi(getSpecificStringCloseDetail2(WholeString, OPPS_NAME.substr(1,OPPS_NAME.length()-1), 8).c_str()) - 1;
        opps.correct_rate = stod(getSpecificStringCloseDetail2(WholeString, OPPS_NAME.substr(1, OPPS_NAME.length() - 1), 4).c_str());
        opps.organization = getSpecificStringCloseDetail(WholeString, ORGANI_KEYWORD1, ORGANI_KEYWORD2);
}

void parseAll() {
        parseUserInfo();
        parseOppsInfo();
        parseCompeInfo();
        parseUserOrganInfo();
        parseOppsOrganInfo();
}

void printing() {
        system("cls");
        //cout << endl << endl;
        cout << "─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ " << user.username << " 님의 정보" << "─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ " << endl << endl;
        cout << "전체 랭킹  : " << user.ranking << " 위, 상위 " << (double)(user.ranking * 100) / (double)TOTAL_STUDENT << "%" << endl << endl;
        cout << "제출       : " << user.submission << " 문제" << endl;
        cout << "총 푼 문제 : " << user.solved_problem_num << " 문제" << endl;
        cout << "맞았습니다 : " << user.corrected << " 문제" << endl;
        cout << "틀렸습니다 : " << user.wrong << " 문제" << endl;
        if (user.correct_rate != 0) {
                cout << "정답률     : " << user.correct_rate << "%" << endl << endl;
                cout << user.organization << " 내에서 현재 " << user.subranking << " 위" << endl << endl;
        }
        else cout << endl;
        

        cout << "─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ " << opps.username << " 님의 정보" << "─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ " << endl << endl;
        cout << "전체 랭킹  : " << opps.ranking << " 위, 상위 " << (double)(opps.ranking * 100) / (double)TOTAL_STUDENT << "%" << endl << endl;
        cout << "제출       : " << opps.submission << " 문제" << endl;
        cout << "총 푼 문제 : " << opps.solved_problem_num << " 문제" << endl;
        cout << "맞았습니다 : " << opps.corrected << " 문제" << endl;
        cout << "틀렸습니다 : " << opps.wrong << " 문제" << endl;
        if (opps.correct_rate != 0) {
                cout << "정답률     : " << opps.correct_rate << "%" << endl << endl;
                cout << opps.organization << " 내에서 현재 " << opps.subranking << " 위" << endl << endl;
        }
        else cout << endl;

        cout << "─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ " << user.username<<"님과 "<<opps.username << " 님 비교" << "─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ " << endl << endl;
        cout << "전체 랭킹          : ";
        if (user.ranking > opps.ranking)
                cout << opps.username << "님이 " << user.ranking - opps.ranking << " 명 만큼 앞서고 있습니다."  << endl;
        else if (user.ranking < opps.ranking)
                cout << user.username << "님이 " << opps.ranking - user.ranking << " 명 만큼 앞서고 있습니다." << endl;
        cout << "총 푼 문제         : ";
        if (user.solved_problem_num > opps.solved_problem_num)
                cout << user.username << "님이 " << user.solved_problem_num - opps.solved_problem_num << " 문제 만큼 앞서고 있습니다."  << endl;
        else if (user.solved_problem_num < opps.solved_problem_num)
                cout << opps.username << "님이 " << opps.solved_problem_num - user.solved_problem_num << " 문제 만큼 앞서고 있습니다."  << endl;
        else
                cout << "두 명의 푼 문제 수가 같습니다." << endl;
        cout << "두 명 모두 푼 문제 : " << user.common_solved_problem_num << " 문제" << endl  << endl;
        cout << user.username << " \t님만 푼 문제 : " << user.solved_problem_num - user.common_solved_problem_num << " 문제" << endl;
        cout << opps.username << " \t님만 푼 문제 : " << opps.solved_problem_num - opps.common_solved_problem_num << " 문제" << endl;
        cout << endl << endl;
        //if (opps.correct_rate != 0) {
        //      cout << "정답률     : " << opps.correct_rate << "%" << endl << endl;
        //      cout << opps.organization << " 내에서 현재 " << opps.subranking << " 위" << endl << endl;
        //}
}

int main(void) {
        while (true) {
                system("cls");
                cout << "자신의 닉네임을 입력하세요 \t\t: ";
                cin >> USER_NAME;
                cout << "비교할 상대의 닉네임을 입력하세요 \t: ";
                cin >> OPPS_NAME;
                INITIAL_SETTING_FOR_OPTIMIZING();
                downloadAll();
                parseAll();
                printing();
                system("pause");
        }
}