#include <stdexcept>
#include "StringUtil.h"
#include "Assembler.h"

static std::unordered_map<std::string, Keyword> s_KeyWords = {
    { "CLS",  Keyword::CLS  },
    { "RET",  Keyword::RET  },
    { "SYS",  Keyword::SYS  },
    { "JP",   Keyword::JP   },
    { "CALL", Keyword::CALL },
    { "SE",   Keyword::SE   },
    { "SNE",  Keyword::SNE  },
    { "LD",   Keyword::LD   },
    { "ADD",  Keyword::ADD  },
    { "OR",   Keyword::OR   },
    { "XOR",  Keyword::XOR  },
    { "AND",  Keyword::AND  },
    { "SUB",  Keyword::SUB  },
    { "SHL",  Keyword::SHL  },
    { "SHR",  Keyword::SHR  },
    { "SUBN", Keyword::SUBN },
    { "RND",  Keyword::RND  },
    { "DRW",  Keyword::DRW  },
    { "SKP",  Keyword::SKP  },
    { "SKNP", Keyword::SKNP },

    { "V0", Keyword::V0 },
    { "V1", Keyword::V1 },
    { "V2", Keyword::V2 },
    { "V3", Keyword::V3 },
    { "V4", Keyword::V4 },
    { "V5", Keyword::V5 },
    { "V6", Keyword::V6 },
    { "V7", Keyword::V7 },
    { "V8", Keyword::V8 },
    { "V9", Keyword::V9 },
    { "VA", Keyword::VA },
    { "VB", Keyword::VB },
    { "VC", Keyword::VC },
    { "VD", Keyword::VD },
    { "VE", Keyword::VE },
    { "VF", Keyword::VF },
    { "DT", Keyword::DT },
    { "ST", Keyword::ST },
    { "I",  Keyword::I },
    
    { "K",  Keyword::K },
    { "F",  Keyword::F },
    { "B",  Keyword::B }
};

// TODO: Write custom tokenizing function instead of strtok
// TODO: Handle expressions in parentheses

bool TokenizeLine(std::vector<Token>& tokensOut, int nLine, char* str)
{
    const char* delimit = " \t";
    char* token = nullptr;
    char* context = nullptr;
    char* temp = str;
    int   tokens = 0;

    while (*temp != '\0')
    {
        if (*temp == ',')
            *temp = ' ';
        ++temp;
    }

    token = strtok_s(str, delimit, &context);
    while (token != NULL)
    {
        Token tok(token);
        std::string normalized = ToUpper(tok.text);

        /* Figure out what kind of token it is */
        if (token[0] == '.')
        {
            tok.type = Token::Type::Directive;
        }
        else if (token[0] == '#')
        {
            try
            {
                tok.value = std::stoi(tok.text.substr(1), nullptr, 0);
            }
            catch (const std::invalid_argument&)
            {
                printf("ERROR at line %d: invalid immediate value '%s'\n", nLine, tok.text.c_str());
                return false;
            }
            tok.type = Token::Type::Immediate;
        }
        else if (token[tok.text.length() - 1] == ':')
        {
            tok.text.erase(tok.text.length() - 1);
            tok.type = Token::Type::Label;
        }
        else if (s_KeyWords.count(normalized) != 0)
        {
            tok.text = normalized;
            tok.type = Token::Type::Keyword;
            tok.keyword = s_KeyWords.at(normalized);
        }

        tok.line = nLine;
        tokensOut.push_back(tok);
        token = strtok_s(nullptr, delimit, &context);
        ++tokens;
    }

    return true;
}

/**
 * Break down a block of code into a vector of tokens
 * @param  tokensOut Vector to receive tokens
 * @param  str       String containing code
 * @return True on success, false otherwise
 */
bool TokenizeCode(std::vector<Token>& tokensOut, const std::string& str)
{
    char* codeBuffer = new char[str.length() + 1];
    strcpy_s(codeBuffer, str.length() + 1, str.c_str());

    const char* delimit = "\r\n";
    char* line = nullptr;
    char* context = nullptr;
    int   nLine = 0;

    line = strtok_s(codeBuffer, delimit, &context);
    while (line != NULL)
    {
        TokenizeLine(tokensOut, nLine, line);
        line = strtok_s(nullptr, delimit, &context);
        ++nLine;
    }

    delete[] codeBuffer;
    return true;
}