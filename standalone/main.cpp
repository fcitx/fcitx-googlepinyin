#include "pinyinime.h"
#include "dictdef.h"
#include <iostream>
#include <iconv.h>
#include <string>
#include <string.h>
#define RET_BUF_LEN 256

using namespace std;
using namespace ime_pinyin;

static char16 retbuf[RET_BUF_LEN];
static char16 (*predict_buf)[kMaxPredictSize + 1] = NULL;
static size_t predict_len;

int main(int argc, char *argv[])
{
    if (argc < 3)
        return 0;
    im_open_decoder(argv[1], argv[2]);
    string s;
    cin >> s ;
    char ubuf[4096];

    iconv_t conv = iconv_open("utf8", "utf16");
    while (true)
    {
        int len = im_search(s.c_str(), s.length());
        for (int i = 0 ;i < len ; i ++)
        {
            char16 *p;
            char *pp = ubuf;
            p = im_get_candidate(i, retbuf, 1024);
            size_t a = utf16_strlen(p) * 2,b = 4096;
            cout << a;
            memset(ubuf, 0 , 4096);
            iconv(conv, (char**) &p, &a, &pp, &b);
            cout << ubuf << endl;
        }
        int cand;
        cin >> cand;
//      if (cand == -1)
//          im_delsearch();
        im_choose(cand);
    }
    im_close_decoder();
}
// kate: indent-mode cstyle; space-indent on; indent-width 0; 
