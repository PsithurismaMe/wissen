/*
    Wissen is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
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

int NumOfquestions{4};
size_t refreshRate{16666};

// RGB array
int rgbHighlighted[] = {10, 13, 11, 12, 13, 3};
int rgb[] = {4, 16, 5, 15, 16, 1};

// Needed to getline wchar_t correctly
std::wstring widen(const std::string &utf8_string)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
    return convert.from_bytes(utf8_string);
}

// Used for debugging
namespace debug
{
    int emptyInts[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

// A struct for storing a word regardless of language
struct word
{
    std::wstring German;
    std::wstring English;
    word(std::wstring _e, std::wstring _deutsch)
    {
        German = _deutsch;
        English = _e;
    }
};

// I dont know why i cant put this in the conjucation namespace without compiler errors
// Remember to free this after use
wchar_t *convertToWideStr(wchar_t letter)
{
    wchar_t *thingToReturn = (wchar_t *)std::malloc(2 * sizeof(wchar_t));
    thingToReturn[0] = letter;
    thingToReturn[1] = '\0';
    return thingToReturn;
}

// A function which returns a vector of unique numbers. End number is not included.
std::vector<signed long int> randomUniqueNum(signed long int start, signed long int end, size_t numberOfEntries)
{
    std::vector<signed long int> thingToReturn;
    while (thingToReturn.size() != numberOfEntries)
    {
        while (1)
        {
            signed long int i = std::rand() % (1 + end - start) - start;
            bool isUnique{1};
            for (auto k : thingToReturn)
            {
                if (k == i)
                {
                    isUnique = 0;
                    break;
                }
            }
            if (isUnique)
            {
                thingToReturn.push_back(i);
                break;
            }
        }
    }
    return thingToReturn;
}

namespace multiChoice
{

    int awnserCache{0};
    float accuracyCache1{1.00};
    float accuracyCache2{1.00};
    // timestamps
    std::array<std::chrono::duration<float>, 20> timeStamps;
    std::uint64_t timeStampIterator = 0;
    // The draw function draws all things on screen. This should only be called when constructing a std::thread
    void draw(int *living, std::wstring title, std::vector<signed long int> *randoms, int *highlighedIndex, std::vector<word> *contents, int maY, int maX, int *correct, int *total, std::chrono::_V2::steady_clock::time_point *start)
    {
        // Set this to 1 to show debug info on questonare
        int debugMode{0};
        while (*living == -1)
        {
            clear();
            attron(COLOR_PAIR(2));
            addwstr(title.c_str());
            attroff(COLOR_PAIR(2));
            printw("\n\n\t");
            for (int i = 0; i < randoms->size(); i++)
            {
                if (*highlighedIndex == i)
                {
                    attron(COLOR_PAIR(13));
                    addwstr((*contents)[(*randoms)[i]].German.c_str());
                    attroff(COLOR_PAIR(13));
                    printw("\n\t");
                }
                else
                {
                    attron(COLOR_PAIR(16));
                    addwstr((*contents)[(*randoms)[i]].German.c_str());
                    attroff(COLOR_PAIR(16));
                    printw("\n\t");
                }
            }
            // Compute duration from start here
            std::chrono::duration<float> elapsedTime = (std::chrono::steady_clock::now() - *start);
            if (debugMode)
            {
                mvprintw(maY - 4, 0, "Value of myInt: %d", debug::emptyInts[1]);
                mvprintw(maY - 3, 0, "Index of last entry: %d", debug::emptyInts[0]);
            }
            mvprintw(maY - 4, 0, "Awnser to previous question: ");
            addwstr((*contents)[awnserCache].German.c_str());
                if (timeStamps.size() != 0)
                {
                    mvprintw(maY - 3, 0, "Duration of last translation: %s sec", std::to_string(timeStamps[((timeStampIterator - 1) % 20)].count()).c_str());
                    float mean {0.0};
                    float variance {0.0};
                    float stdDeviation {0.0};
                    for (auto i : timeStamps)
                    {
                        mean = i.count();
                    }
                    for (auto i : timeStamps)
                    {
                        variance += std::pow((((float) i.count() - mean)), 2);
                    }
                    variance = variance / (float) timeStamps.size();
                    stdDeviation = std::sqrt(variance);
                    mvprintw(maY - 2, 0, "Standard Deviation: %.3f", stdDeviation);

                }
            mvprintw(maY - 1, 0, "%s sec", std::to_string(elapsedTime.count()).c_str());

            // Display accuracy
            {
                float accuracy = 100 * ((float)*correct / (float)*total);
                if (accuracyCache2 < accuracyCache1)
                {
                    attron(COLOR_PAIR(4));
                    mvprintw(maY - 1, maX / 2, "Accuracy: %.2f%%", accuracy);
                    attroff(COLOR_PAIR(4));
                }
                else
                {
                    attron(COLOR_PAIR(5));
                    mvprintw(maY - 1, maX / 2, "Accuracy: %.2f%%", accuracy);
                    attroff(COLOR_PAIR(5));
                }
            }

            refresh();
            // set frame times here
            std::this_thread::sleep_for(std::chrono::microseconds(refreshRate));
        }
    }

    int individualWordTranslationMultipleChoice(std::vector<word> *contents, std::vector<signed long int> *randoms, std::wstring title, int *correct, int *total)
    {
        // Hide user input
        noecho();

        // Hide the cursor
        curs_set(0);
        int maX;
        int maY;
        getmaxyx(stdscr, maY, maX);
        int choice{-1};
        int highlighedIndex{0};
        // Record starting time
        std::chrono::_V2::steady_clock::time_point start = std::chrono::steady_clock::now();

        // start drawing thread
        std::thread drawing(multiChoice::draw, &choice, title, randoms, &highlighedIndex, contents, maY, maX, correct, total, &start);
        while (choice == -1)
        {
            // Wait for user to press a key
            int response = getch();
            switch (response)
            {
            case (KEY_UP):
            {
                if (highlighedIndex > 0)
                {
                    highlighedIndex--;
                }
                else
                {
                    highlighedIndex = (randoms->size() - 1);
                }
                break;
            }
            case (KEY_DOWN):
            {
                if (highlighedIndex < randoms->size() - 1)
                {
                    highlighedIndex++;
                }
                else
                {
                    highlighedIndex = 0;
                }
                break;
            }
            case ('q'):
            {
                choice = -2;
                drawing.join();
                break;
            }
            case ('Q'):
            {
                choice = -2;
                drawing.join();
                break;
            }
            case ('\n'):
            {
                choice = highlighedIndex;
                drawing.join();
                timeStamps.at(timeStampIterator % 20) = ((std::chrono::steady_clock::now() - start)); // Appears to break user input
                timeStampIterator++;
                break;
            }
            default:
                break;
            }
        }
        return choice;
    }
    // This function initiates multiple choice quisonare
    void startRandomChoice(std::vector<word> *masterKey, int questions)
    {
        int correct{0};
        int total{0};
        char run{1};
        while (run)
        {
            // Generate 4 uniqe random numbers between 0 and 4
            std::vector<signed long int> randomWordIndexes = randomUniqueNum(0, masterKey->size() - 1, questions);
            // Pick a random number between 0 and 4
            int myInt = std::rand() % randomWordIndexes.size();
            debug::emptyInts[1] = myInt;

            std::wstring title = L"Please translate the following: " + masterKey->at(randomWordIndexes.at(myInt)).English;
            // Start quizonare and record awnser
            int userAwnser = multiChoice::individualWordTranslationMultipleChoice(masterKey, &randomWordIndexes, title, &correct, &total);
            accuracyCache1 = 100 * ((float)correct / (float)total);
            debug::emptyInts[0] = userAwnser;
            awnserCache = randomWordIndexes[myInt];
            // Check if awnser is correct
            if (userAwnser == myInt)
            {
                correct++;
            }
            else if (userAwnser == -2)
            {
                run = 0;
            }
            total++;
            accuracyCache2 = 100 * ((float)correct / (float)total);
        }
    }
    void readMasterKey(std::vector<word> &target)
    {

        std::ifstream file("MasterKey.csv", std::ios::in);
        if (!file.is_open())
        {
            std::cerr << "Failed to read file." << std::endl;
            return;
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
                target.push_back(word(buffers.at(1), buffers.at(0)));
            }
        }
        file.close();
    }

}

namespace conjucation
{
    void printWideWithAttribute(int &indicator, int index, int colorpairOn, int colorpairOff, std::array<std::wstring, 6> &inputBuffers)
    {
        if (indicator == index)
        {
            attron(COLOR_PAIR(colorpairOn));
            addwstr(inputBuffers[index].c_str());
            attroff(COLOR_PAIR(colorpairOn));
            printw("\n");
        }
        else
        {
            attron(COLOR_PAIR(colorpairOff));
            addwstr(inputBuffers[index].c_str());
            attroff(COLOR_PAIR(colorpairOff));
            printw("\n");
        }
    }

    struct verb
    {
        std::wstring infinitive;
        std::wstring translation;
        std::array<std::wstring, 6> simplePresentTense;
        std::array<std::wstring, 6> simplePastTense;
        std::array<std::wstring, 6> simpleFutureTense;
        std::array<std::wstring, 6> * getTense(int tense) // 0 == Present, 1 == Past, 2 == Future
        {
            if (tense == 0)
            {
                return &simplePresentTense;
            }
            if (tense == 1)
            {
                return &simplePastTense;
            }
            if (tense == 2)
            {
                return &simpleFutureTense;
            }
            // This should never happen
            return &simplePresentTense;
        }
    };
    //  read files
    void readData(std::vector<verb> &subject)
    {
        std::ifstream file("Conjugation.csv", std::ios::in);
        if (!file.is_open())
        {
            std::cerr << "Failed to read file." << std::endl;
            return;
        }
        {
            // Skip the first line since its just comments
            std::string dummyBuffer;
            std::getline(file, dummyBuffer);
        }
        while (!file.eof())
        {
            std::string entireLineContents;
            std::getline(file, entireLineContents);
            std::wstring wideEntireLineContents = widen(entireLineContents);
            size_t i{1};
            {
                std::array<std::wstring, 20> buffers;
                for (int x = 0; x < buffers.size(); x++)
                {
                    while (wideEntireLineContents.at(i) != L'"')
                    {
                        buffers.at(x) += wideEntireLineContents.at(i);
                        i++;
                    }
                        i += 3;
                    
                }
                subject.push_back(verb());
                subject.at(subject.size()-1).translation = buffers.at(0);
                subject.at(subject.size()-1).infinitive = buffers.at(1);
                subject.at(subject.size()-1).simplePresentTense.at(0) = buffers.at(2);
                subject.at(subject.size()-1).simplePresentTense.at(1) = buffers.at(3);
                subject.at(subject.size()-1).simplePresentTense.at(2) = buffers.at(4);
                subject.at(subject.size()-1).simplePresentTense.at(3) = buffers.at(5);
                subject.at(subject.size()-1).simplePresentTense.at(4) = buffers.at(6);
                subject.at(subject.size()-1).simplePresentTense.at(5) = buffers.at(7);
                subject.at(subject.size()-1).simplePastTense.at(0) = buffers.at(8);
                subject.at(subject.size()-1).simplePastTense.at(1) = buffers.at(9);
                subject.at(subject.size()-1).simplePastTense.at(2) = buffers.at(10);
                subject.at(subject.size()-1).simplePastTense.at(3) = buffers.at(11);
                subject.at(subject.size()-1).simplePastTense.at(4) = buffers.at(12);
                subject.at(subject.size()-1).simplePastTense.at(5) = buffers.at(13);
                subject.at(subject.size()-1).simpleFutureTense.at(0) = buffers.at(14);
                subject.at(subject.size()-1).simpleFutureTense.at(1) = buffers.at(15);
                subject.at(subject.size()-1).simpleFutureTense.at(2) = buffers.at(16);
                subject.at(subject.size()-1).simpleFutureTense.at(3) = buffers.at(17);
                subject.at(subject.size()-1).simpleFutureTense.at(4) = buffers.at(18);
                subject.at(subject.size()-1).simpleFutureTense.at(5) = buffers.at(19);
    
            }
        }
        file.close();
    }
    void start(std::vector<verb> &key)
    {
        int correct{0};
        int total{0};
        char run{1};
        noecho();
        curs_set(0);
        while (run)
        {
            char drawing{1};
            int activeIndex{0};
            int randomInfinitive = std::rand() % key.size();
            int tense = std::rand() % 3;
            std::array<std::string, 3> names = {"present", "past", "future"};
            std::array<std::wstring, 6> * conjucationKey = key.at(randomInfinitive).getTense(tense);
            std::array<std::wstring, 6> inputBuffers;
            while (drawing)
            {
                clear();
                int x;
                int y;
                getmaxyx(stdscr, y, x);
                printw("Verb: ");
                addwstr(key.at(randomInfinitive).infinitive.c_str());
                printw("\nTranslation: ");
                addwstr(key.at(randomInfinitive).translation.c_str());
                printw("\nConjugate in %s tense", names.at(tense).c_str());
                printw("\nAwnsers cannot contain caps. Press + to access special characters");
                printw("\n\n\n");
                printWideWithAttribute(activeIndex, 0, 13, 16, inputBuffers);
                printWideWithAttribute(activeIndex, 1, 13, 16, inputBuffers);
                printWideWithAttribute(activeIndex, 2, 13, 16, inputBuffers);
                printWideWithAttribute(activeIndex, 3, 13, 16, inputBuffers);
                printWideWithAttribute(activeIndex, 4, 13, 16, inputBuffers);
                printWideWithAttribute(activeIndex, 5, 13, 16, inputBuffers);
                mvprintw(y - 1, 0, "Press - to submit for grading");
                refresh();
                int response = getch();
                switch (response)
                {
                case (KEY_UP):
                {
                    if (activeIndex < 1)
                    {
                        activeIndex = 5;
                    }
                    else
                    {
                        activeIndex--;
                    }
                }
                break;
                case (KEY_DOWN):
                {
                    if (activeIndex > 4)
                    {
                        activeIndex = 0;
                    }
                    else
                    {
                        activeIndex++;
                    }
                }
                break;
                case ('\n'):
                {
                    if (activeIndex > 4)
                    {
                        activeIndex = 0;
                    }
                    else
                    {
                        activeIndex++;
                    }
                }
                break;
                case (KEY_BACKSPACE):
                {
                    if (inputBuffers[activeIndex].size() > 0)
                    {
                        inputBuffers[activeIndex].pop_back();
                    }
                }
                break;
                case ('+'):
                {
                    char hasResponded{0};
                    int selected{0};
                    std::array<std::wstring, 7> choices;
                    choices[0] = L"ä";
                    choices[1] = L"ö";
                    choices[2] = L"ü";
                    choices[3] = L"ß";
                    choices[4] = L"Ä";
                    choices[5] = L"Ö";
                    choices[6] = L"Ü";
                    while (!hasResponded)
                    {
                        clear();
                        mvprintw(2, 0, "Choose using LEFT/RIGHT/ENTER:\t");
                        for (int k = 0; k < 7; k++)
                        {
                            if (k == selected)
                            {
                                attron(COLOR_PAIR(12));
                                addwstr(choices[k].c_str());
                                attroff(COLOR_PAIR(12));
                                printw("  ");
                            }
                            else
                            {
                                attron(COLOR_PAIR(15));
                                addwstr(choices[k].c_str());
                                attroff(COLOR_PAIR(15));
                                printw("  ");
                            }
                        }
                        refresh();
                        int suss = getch();
                        switch (suss)
                        {
                        case (KEY_RIGHT):
                        {
                            if (selected < 6)
                            {
                                selected++;
                            }
                            else
                            {
                                selected = 0;
                            }
                        }
                        break;
                        case (KEY_LEFT):
                        {
                            if (selected > 0)
                            {
                                selected--;
                            }
                            else
                            {
                                selected = 6;
                            }
                        }
                        break;
                        case ('\n'):
                        {
                            hasResponded = 1;
                            inputBuffers[activeIndex] += choices[selected];
                        }
                        break;
                        case ('q'):
                        {
                            hasResponded = 1;
                        }
                        break;
                        default:
                            break;
                        }
                    }
                }
                break;
                case ('-'):
                {
                    
                    std::vector<std::string> corrections;
                    size_t correct {0};
                    size_t total {0};
                    // conjucations.at(0)
                    for (int x = 0; x < 6; x++)
                    {
                        std::string accuracy;
                        total += conjucationKey->at(x).size();
                        for (size_t k = 0; k < inputBuffers.at(x).size() || k < conjucationKey->at(x).size(); k++)
                        {
                            try
                            {
                                if (inputBuffers.at(x).at(k) == conjucationKey->at(x).at(k))
                                {
                                    correct++;
                                    accuracy += '1';
                                }
                                else
                                {
                                    accuracy += '0';
                                }
                            }
                            catch(...)
                            {
                                accuracy += '0';
                            }
                            
                        }
                        corrections.push_back(accuracy);
                    }
                    
                    // Print corrections
                    char stop{0};
                    while (!stop)
                    {
                        clear();
                        printw("Verb: ");
                        addwstr(key.at(randomInfinitive).infinitive.c_str());
                        printw("\nTranslation: ");
                        addwstr(key.at(randomInfinitive).translation.c_str());
                        printw("\nConjugated in %s tense", names.at(tense).c_str());
                        printw("\nAwnser key is shown\n");
                        printw("\n\n\n");
                        {
                            
                            for (int x = 0; x < 6; x++)
                            {
                                size_t i{0};

                                for (wchar_t k : conjucationKey->at(x))
                                {
                                    wchar_t *freeMe = convertToWideStr(k);
                                    if ((i <= corrections[x].size()))
                                    {
                                        if (corrections[x][i] == '1')
                                        {
                                            attron(COLOR_PAIR(11));
                                            addwstr(freeMe);
                                            attroff(COLOR_PAIR(11));
                                        }
                                        else
                                        {
                                            attron(COLOR_PAIR(10));
                                            addwstr(freeMe);
                                            attroff(COLOR_PAIR(10));
                                        }
                                    }
                                    else
                                    {
                                        attron(COLOR_PAIR(10));
                                        addwstr(freeMe);
                                        attroff(COLOR_PAIR(10));
                                    }
                                    i++;
                                    std::free((void *)freeMe);
                                    
                                }
                                addch('\n');
                            }
                        }
                        mvprintw(y - 2, 0, "Score: %.2f%%", ((float)correct / (float)total) * 100);
                        mvprintw(y - 1, 0, "Press q to exit");
                        refresh();
                        int u = getch();
                        if (u == 'q' || u == 'Q')
                        {
                            stop = 1;
                            break;
                        }
                    }
                }
                    drawing = 0;
                    break;
                case ('`'):
                    drawing = 0;
                    run = 0;
                    break;
                default:
                {
                    if (response > 91 && response < 123)
                    {
                        inputBuffers[activeIndex] += response;
                    }
                }
                break;
                }
            }
        }
    }
}

// Unimplemented
namespace sentenceTranslation
{

}
namespace titleScreen
{
    int pos{0};
    char quitProgram{0};
    void start()
    {
        std::array<std::string, 2> title;
        std::array<std::string, 3> choices;
        title[0] = "German";
        title[1] = "Quizzer";
        choices[0] = "Multiple Choice Word Translation";
        choices[1] = "Verb Conjugation";
        choices[2] = "Exit";
        std::string license = "Wissen is licensed under the GPL-v3. See LICENSE file for more details.";


        while (!quitProgram)
        {
            clear();
            int x;
            int y;
            getmaxyx(stdscr, y, x);
            mvprintw(1, ((x / 2) - (title[0].length() / 2)), "%s", title[0].c_str());
            mvprintw(2, ((x / 2) - (title[1].length() / 2)), "%s\n\n\n", title[1].c_str());

            for (int i = 0; i < choices.size(); i++)
            {
                if (pos == i)
                {
                    attron(A_REVERSE);
                    mvprintw(5 + i, ((x / 2) - (choices[i].length() / 2)), "%s", choices[i].c_str());
                    attroff(A_REVERSE);
                }
                else
                {
                    mvprintw(5 + i, ((x / 2) - (choices[i].length() / 2)), "%s", choices[i].c_str());
                }
            }
            mvprintw(y - 1, ((x / 2) - (license.length() / 2)), "%s", license.c_str());
            refresh();
            int response = getch();
            switch (response)
            {
            case (KEY_UP):
                if (pos == 0)
                {
                    pos = choices.size() - 1;
                }
                else
                {
                    pos--;
                }
                break;
            case (KEY_DOWN):
                if (pos == choices.size() - 1)
                {
                    pos = 0;
                }
                else
                {
                    pos++;
                }
                break;
            case ('\n'):
            {
                switch (pos)
                {
                case (0):
                {
                    std::vector<word> masterKey;
                    multiChoice::readMasterKey(masterKey);
                    multiChoice::startRandomChoice(&masterKey, NumOfquestions);
                }
                break;
                case (1):
                {
                    std::vector<conjucation::verb> verbs;
                    conjucation::readData(verbs);
                    conjucation::start(verbs);
                }
                break;
                case (2):
                {
                    quitProgram = 1;
                }
                break;
                default:
                    break;
                }
            }
            break;
            default:
                break;
            }
        }
    }
}
int main(int argc, char **argv)
{
    if (argc > 1 && !std::isdigit(*argv[1]))
    {
        std::cout << "Ultimate Quizzer- A tool to help learn German written in C++" << std::endl;
        std::cout << "Command syntax:\n\ta.out <refresh rate> <number of multiple choice questions>\n\nAll arguments are optional" << std::endl;
        return 0;
    }
    if (argc > 1 && std::isdigit(*argv[1]))
    {
        double placeholder = 1.0 / ((double)std::atoi(argv[1]));
        refreshRate = 1000000 * placeholder;
        if (argc > 2 && std::isdigit(*argv[2]))
        {
            NumOfquestions = std::atoi(argv[2]);
        }
    }
    // Seed random number generator
    std::srand(std::time(0));
    // Needed for essets to be printed correctly
    setlocale(LC_ALL, "");
    std::setlocale(LC_ALL, nullptr);

    // Initialize ncurses
    initscr();

    // Give ncurses more control over keypresses
    keypad(stdscr, TRUE);

    // Disable CTRL-C to kill
    cbreak();

    // Check if terminal supports colors
    if (has_colors() == FALSE)
    {
        endwin();
        std::cerr << "Terminal does not support color. Exiting" << std::endl;
        return -1;
    }

    // Define color pairs
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // Unselected option
    init_pair(2, COLOR_CYAN, COLOR_BLACK);   // UI Color
    init_pair(3, COLOR_BLACK, COLOR_YELLOW); // Selected option
    init_pair(4, COLOR_RED, COLOR_BLACK);    // Bad
    init_pair(5, COLOR_GREEN, COLOR_BLACK);  // Good
    // RGB color pairs
    init_pair(10, COLOR_BLACK, COLOR_RED);     // Red highlighted
    init_pair(11, COLOR_BLACK, COLOR_GREEN);   // Green highlighted
    init_pair(12, COLOR_BLACK, COLOR_BLUE);    // Blue highlighted
    init_pair(13, COLOR_BLACK, COLOR_MAGENTA); // Magenta highlighted
    init_pair(14, COLOR_BLACK, COLOR_WHITE);   // Monochrome highlighted
    init_pair(15, COLOR_BLUE, COLOR_BLACK);    // Blue not highlighted
    init_pair(16, COLOR_MAGENTA, COLOR_BLACK); // Magenta not highlighted
    init_pair(17, COLOR_WHITE, COLOR_BLACK);
    titleScreen::start();
    endwin();
    return 0;
}
