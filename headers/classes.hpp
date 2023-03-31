#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <ctime>
#include <chrono>
#include <clocale>
#include <locale>
#include <codecvt>
#include <string>
#include <ncurses.h>
#include <thread>
#include <random>

namespace wissen
{
    std::wstring widen(const std::string &utf8_string)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
        return convert.from_bytes(utf8_string);
    }
    class word
    {
    protected:
        std::wstring german;
        std::wstring english;
        char shouldAppearInQuiz;
    public:
        char getVisibility()
        {
            return shouldAppearInQuiz;
        }
        void setVisibility(char v)
        {
            shouldAppearInQuiz = v;
        }
        word(std::wstring _e, std::wstring _deutsch)
        {
            german = _deutsch;
            english = _e;
            shouldAppearInQuiz = ' ';
        }
        void setGerman(std::wstring &deutsch)
        {
            german = deutsch;
        }
        void setEnglish(std::wstring &engl)
        {
            english = engl;
        }
        std::wstring &getGerman()
        {
            return german;
        }
        std::wstring &getEnglish()
        {
            return english;
        }
        word()
        {

        }
    };
    class quizzer
    {
    protected:
        std::vector<word> masterKey;
        std::vector<word *> wordsToQuiz;

    public:
        quizzer()
        {
            std::ifstream file("MasterKey.csv", std::ios::in);
            if (!file.is_open())
            {
                std::cerr << "Failed to read file." << std::endl;
                return;
            }
            {
                // Skip the first line
                std::string dummyBuffer;
                std::getline(file, dummyBuffer);
            }
            while (!file.eof())
            {
                std::string entireLineContents;
                std::getline(file, entireLineContents);
                // Now convert the entire file into a wchar_t string
                std::wstring wideEntireLineContents = widen(entireLineContents);
                // Lastly, parse the line
                size_t i{1};
                {
                    std::array<std::wstring, 2> buffers;
                    while (wideEntireLineContents.at(i) != L'"')
                    {
                        buffers.at(0) += wideEntireLineContents.at(i);
                        i++;
                    }
                    i += 3;
                    while (wideEntireLineContents.at(i) != L'"')
                    {
                        buffers.at(1) += wideEntireLineContents.at(i);
                        i++;
                    }
                    masterKey.push_back(word(buffers.at(1), buffers.at(0)));
                }
            }
            file.close();
        }
        void pickVocab()
        {
            bool isInMenu {1};
            int highlightedIndex {0};
            size_t words = masterKey.size();
            while (isInMenu)
            {
                clear();
                int maxX, maxY;
                getmaxyx(stdscr, maxY, maxX);
                for (int i = highlightedIndex; i < highlightedIndex + maxY && i < masterKey.size(); i++)
                {
                    i == highlightedIndex ? attron(A_REVERSE) : attroff(A_REVERSE);
                    printw("[%c] ", masterKey.at(i).getVisibility());
                    addwstr(masterKey.at(i).getGerman().c_str());
                    printw(" | ");
                    addwstr(masterKey.at(i).getEnglish().c_str());
                    addch('\n');
                }
                refresh();
                int response = getch();
                switch (response)
                {
                case (KEY_UP):
                    highlightedIndex > 0 ? highlightedIndex-- : highlightedIndex = words - 1;
                    break;
                case (KEY_DOWN):
                    highlightedIndex < words - 1 ? highlightedIndex++ : highlightedIndex = 0;
                    break;
                case (' '):
                    if (masterKey.at(highlightedIndex).getVisibility() == ' ')
                    {
                        wordsToQuiz.push_back(&masterKey.at(highlightedIndex));
                        masterKey.at(highlightedIndex).setVisibility('X');
                    }
                    else
                    {
                        for (size_t k = 0; k < wordsToQuiz.size(); k++)
                        {
                            if (wordsToQuiz.at(k) == &masterKey.at(highlightedIndex))
                            {
                                wordsToQuiz.at(k) = wordsToQuiz.at(wordsToQuiz.size() - 1);
                                wordsToQuiz.pop_back();
                                masterKey.at(highlightedIndex).setVisibility(' ');
                                break;
                            }
                        }
                    }
                    break;
                case ('a'):
                    if (wordsToQuiz.size() > 0)
                    {
                        for (size_t k = 0; k < wordsToQuiz.size(); k++)
                        {
                            wordsToQuiz.at(k)->setVisibility(' ');
                        }
                        wordsToQuiz.clear();
                    }
                    else
                    {
                        wordsToQuiz.clear();
                        for (size_t k = 0; k < masterKey.size(); k++)
                        {
                            wordsToQuiz.push_back(&masterKey.at(k));
                            wordsToQuiz.at(wordsToQuiz.size() - 1)->setVisibility('X');
                        }
                    }
                    break;
                case ('`'):
                    isInMenu = 0;
                    break;
                default:
                    break;
                }
            }
            
        }
        void startQuizonare()
        {
            
        }
    };
}