#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>

using namespace std;

struct state
{
    string input;
    string state_term;
    string next_state;
    string output;
    bool remove = false;
};

struct simple_tmp
{
    string input[4];
    bool compatible = true;
    vector<vector<int>> position;
};

int get_position(set<string>, string);
string get_input(set<string>, int);
int find(vector<string>, string);

int main(int argc, char *argv[])
{
    if (argc == 4)
    {
        // argv[0] = ./a.out

        ifstream file(argv[1]); // input.kiss

        // argv[2] = output.kiss 輸出kiss檔案名稱
        string output_kiss = argv[2];

        // argv[3] = output.dot 輸出dot檔案名稱
        string output_dot = argv[3];

        if (!file)
            cout << "file not found" << endl;
        else
        {
            string file_content, state_input, state_term_input, state_next_input, first_state, state_output;

            int input_num = 0, output_num = 0, state_num = 0,
                state_convert_num = 0;

            int possible_num = 0, classification = 0, round = -1;

            set<string> all_term;
            state **state_table = NULL;
            simple_tmp **simple_table = NULL;

            // 輸入
            while (!file.eof())
            {
                file >> file_content;
                if (file_content == ".i")
                {
                    file >> input_num;                // 可能為1 or 2
                    possible_num = pow(2, input_num); // input = 2, possible_num => 2^2 = 4
                }
                else if (file_content == ".o")
                {
                    file >> output_num;
                }
                else if (file_content == ".p")
                {
                    file >> state_convert_num; // 16
                }
                else if (file_content == ".s")
                {
                    file >> state_num; // 8
                }
                else if (file_content == ".r")
                {
                    file >> first_state; // a

                    state_table = new state *[possible_num]; // 二維狀態表
                    for (int i = 0; i < possible_num; i++)
                        state_table[i] = new state[state_num]; // 8

                    simple_table = new simple_tmp *[state_num]; // 二維化簡陣列
                    for (int i = 0; i < state_num; i++)
                        simple_table[i] = new simple_tmp[state_num];

                    for (int i = 0; i < state_convert_num; i++)
                    {
                        // 將輸入資料填入表格
                        file >> state_input >> state_term_input >> state_next_input >> state_output; // 0 a a 0,  0 a b 0

                        classification = i % possible_num; // e.g possible_num = 2   010101

                        if (classification == 0)
                            round++;

                        all_term.insert(state_term_input);
                        state_table[classification][round].input = state_input;           // string
                        state_table[classification][round].state_term = state_term_input; // string
                        state_table[classification][round].next_state = state_next_input; // string
                        state_table[classification][round].output = state_output;         // string
                    }

                    // Debug 檢查資料輸入完的狀態表
                    /*for (int i = 0; i < possible_num; i++)
                    {
                        for (int j = 0; j < state_num; j++)
                        {

                            cout << "input : " << state_table[i][j].input << "  "
                                 << "current state term : " << state_table[i][j].state_term << "  "
                                 << "next state : " << state_table[i][j].next_state << "  "
                                 << "output : " << state_table[i][j].output << endl;
                        }
                    }*/
                }
            }

            // 先初步找出一定不相容的term
            for (int i = 0; i < possible_num; i++) // input
            {
                for (int j = 0; j < state_num; j++) // 定位term
                {
                    for (int k = 0; k < state_num; k++) // 遍歷term
                    {
                        if (j != k)
                        {
                            // 一個input的比對只要不一樣就是不相容
                            if (state_table[i][j].output != state_table[i][k].output)
                                simple_table[k][j].compatible = false;
                        }
                    }
                }
            }

            ////Debug 檢查初步的相容性比對狀況
            /*for (int i = 0; i < state_num; i++)
            {
                for (int j = 0; j < state_num; j++)
                {
                    cout << "i: " << i << " " << "j: " << j << " " << simple_table[i][j].compatible;
                    cout << endl;
                }
            } */

            // 檢查next state有沒有機會相容
            for (int i = 0; i < state_num; i++)
            {
                for (int j = i + 1; j < state_num; j++)
                {
                    if (simple_table[j][i].compatible != false)
                    {
                        for (int k = 0; k < possible_num; k++)
                        {

                            simple_table[j][i].input[k] += state_table[k][j].next_state;
                            simple_table[j][i].input[k] += state_table[k][i].next_state;

                            // state term轉成數值，相當於位置
                            vector<int> row;

                            row.push_back(get_position(all_term, state_table[k][j].next_state));
                            row.push_back(get_position(all_term, state_table[k][i].next_state));
                            simple_table[j][i].position.push_back(row);
                        }
                    }
                }
            }

            // debug  檢查化簡表中儲存的位置及狀態
            /*for (int i = 0; i < state_num; i++)
            {
                for (int j = i + 1; j < state_num; j++)
                {
                    cout << "i :" << i << " " << "j: " << j << endl;

                    for (int k = 0; k < simple_table[j][i].input[k].size(); k++)
                    {
                        cout << "k :" << k << " ";
                        cout << simple_table[j][i].input[k] << " ";
                        cout << endl;
                    }
                    cout << endl;

                    for (int z = 0; z < simple_table[j][i].position.size(); z++)
                    {
                        cout << "input" << z << "位置: ";
                        for (int p = 0; p < simple_table[j][i].position[z].size(); p++)
                        {
                            cout << simple_table[j][i].position[z][p] << " ";
                        }
                        cout << endl;
                    }
                    cout << endl;
                }
            }*/

            // 開始用關聯state進行化簡表格的相容性檢查
            bool check = true; // merge or simpefy
            while (check)
            {
                check = false;
                for (int i = 0; i < state_num; i++) // 遍歷表格
                {
                    for (int j = i + 1; j < state_num; j++) // 遍歷表格
                    {
                        if (simple_table[j][i].compatible != false) // 該格有可能為相容才檢查
                        {
                            for (int k = 0; k < simple_table[j][i].position.size(); k++)
                            {
                                // 檢查該格存的位置是否有不相容的位置
                                if (simple_table[simple_table[j][i].position[k][0]]
                                                [simple_table[j][i].position[k][1]]
                                                    .compatible == false)
                                {
                                    // 如果該格有存在不相容的位置，使該格也變成不相容
                                    simple_table[j][i].compatible = false;
                                    simple_table[i][j].compatible = false;
                                    check = true;
                                }
                            }
                        }
                    }
                }
            }

            // debug 查看化簡表所有的相容性
            /*for (int i = 0; i < state_num; i++)
            {
                for (int j = i + 1; j < state_num; j++)
                {
                    cout << "i: " << j << "  " << "j: " << i << endl;
                    cout << "compatible: " << simple_table[j][i].compatible << endl;
                    cout << endl;
                }
                cout << endl;
            }*/

            // 狀態合併
            for (int i = 0; i < state_num; i++)
            {
                for (int j = i + 1; j < state_num; j++)
                {
                    if (simple_table[j][i].compatible != false) // 找可相容的格子進行
                    {
                        int max = 0, min = 0;
                        string replace, change;
                        if (i > j)
                        {
                            max = i;
                            min = j;
                        }
                        else
                        {
                            max = j;
                            min = i;
                        }

                        simple_table[j][i].compatible == false; // 應該可以不用

                        change = state_table[0][min].state_term;
                        replace = state_table[0][max].state_term;

                        // 丟棄索引值較大的
                        for (int k = 0; k < possible_num; k++)
                            state_table[k][max].remove = true;

                        // 遍歷state_table找該term
                        for (int m = 0; m < possible_num; m++)
                        {
                            for (int n = 0; n < state_num; n++)
                            {
                                if (state_table[m][n].next_state == replace)
                                    state_table[m][n].next_state = change;
                            }
                        }
                    }
                }
            }

            // debug 查看合併完的狀態表
            for (int i = 0; i < state_num; i++)
            {
                for (int j = 0; j < possible_num; j++)
                {
                    cout << "input: " << state_table[j][i].input << endl;
                    cout << "j: " << j << " "
                         << "i: " << i << endl;
                    cout << "state: " << state_table[j][i].state_term << endl;
                    cout << "next state: " << state_table[j][i].next_state << endl;
                    cout << "output: " << state_table[j][i].output << endl;
                    cout << "remove: " << state_table[j][i].remove << endl;
                    cout << endl;
                }
                cout << endl;
            }

            // 輸出dot檔案
            fstream dot_out;
            dot_out.open(output_dot, ios::out);
            dot_out << "digraph "
                    << "STG "
                    << "{" << endl;
            dot_out << "    rankdir=LR;" << endl;
            dot_out << endl;
            dot_out << "    INIT [shape=point];" << endl;

            for (int i = 0; i < state_num; i++)
            {
                if (state_table[0][i].remove == false)
                {
                    dot_out << "    " << state_table[0][i].state_term << " ";
                    dot_out << "[label=\"";
                    dot_out << state_table[0][i].state_term;
                    dot_out << "\"];";
                    dot_out << endl;
                }
            }
            dot_out << endl;

            dot_out << "    INIT -> " << first_state << ";" << endl;
            vector<string> unique;      // 存放state用
            vector<string> unique_info; // 存放input與output
            if (input_num == 1)
            {
                for (int i = 0; i < state_num; i++)
                {
                    for (int j = 0; j < possible_num; j++)
                    {
                        if (state_table[j][i].remove == false) //
                        {
                            dot_out << "    " << state_table[j][i].state_term << " ";
                            dot_out << "-> ";
                            dot_out << state_table[j][i].next_state << " ";
                            dot_out << "[label=\"";
                            dot_out << state_table[j][i].input << "/" << state_table[j][i].output;
                            dot_out << "\"];";
                            dot_out << endl;
                        }
                    }
                }
            }
            else if (input_num == 2)
            {
                for (int i = 0; i < state_num; i++)
                {
                    // 遍歷一次表格先將格式處理好
                    for (int j = 0; j < possible_num; j++)
                    {
                        if (state_table[j][i].remove == false)
                        {
                            int pos = 0;
                            pos = find(unique, state_table[j][i].next_state);

                            if (pos == -1) // -1表示沒出現過
                            {
                                // 將新的元素加入
                                unique.push_back(state_table[j][i].next_state);
                                unique_info.push_back(state_table[j][i].input + "/" + state_table[j][i].output);
                            }
                            else // 如果已經出現過，並找到他位於哪一個位置後加到字串的後面
                            {
                                string tmp; //,11/0
                                tmp += ",";
                                tmp += state_table[j][i].input;
                                tmp += "/";
                                tmp += state_table[j][i].output;

                                unique_info[pos] += tmp;
                            }
                            cout << "set:" << endl;
                            for (const auto &h : unique)
                            {
                                cout << h << endl;
                            }
                            cout << "vector:" << endl;
                            for (int g = 0; g < unique_info.size(); g++)
                            {
                                cout << unique_info[g] << endl;
                            }
                        }
                    }

                    // cout << "i: " << i << endl;
                    // cout << "set: " << endl;
                    // for (const auto &f : unique)
                    // {
                    //     cout << f << "  ";
                    // }
                    // cout << endl;
                    // cout << "vector: " << endl;
                    // for (int o = 0; o < unique_info.size(); o++)
                    // {
                    //     cout << "i : " << o << endl;

                    //     cout << unique_info[o] << endl;
                    // }
                    // cout << endl;

                    // 如果i到2，2這一行全部都是false，上面因為有判斷remove的關係
                    // 所以不會有資料被加入到vector及set中，但如果沒有size==0的判斷
                    //  k會是0 state_table[0][i].remove == false 這一格成立 進而去印下面的東西

                    //  一次列印
                    if (unique_info.size() != 0)
                    {
                        for (int k = 0; k < unique_info.size(); k++)
                        {
                            if (state_table[k][i].remove == false)
                            {
                                dot_out << "    " << state_table[k][i].state_term << " ";
                                dot_out << "-> ";
                                dot_out << unique[k];
                                dot_out << " ";
                                dot_out << "[label=\"";
                                dot_out << unique_info[k];
                                dot_out << "\"];";
                                dot_out << endl;
                            }
                        }
                    }

                    // 清空元素

                    unique.clear();
                    vector<string>().swap(unique_info);
                }
            }

            dot_out << "}";

            dot_out.close();

            // 輸出kiss檔案
            fstream kiss_out;
            kiss_out.open(output_kiss, ios::out);

            kiss_out << ".start_kiss" << endl;
            kiss_out << ".i " << input_num << endl;
            kiss_out << ".o " << 1 << endl;

            int non_remove_num = 0;
            for (int i = 0; i < state_num; i++)
            {
                if (state_table[0][i].remove == false)
                    non_remove_num++;
            }
            kiss_out << ".p " << non_remove_num * possible_num << endl;
            kiss_out << ".s " << non_remove_num << endl;
            kiss_out << ".r " << first_state << endl;

            for (int i = 0; i < state_num; i++)
            {
                for (int j = 0; j < possible_num; j++)
                {
                    if (state_table[j][i].remove == false)
                    {
                        kiss_out << state_table[j][i].input << " ";
                        kiss_out << state_table[j][i].state_term << " ";
                        kiss_out << state_table[j][i].next_state << " ";
                        kiss_out << state_table[j][i].output;
                        kiss_out << endl;
                    }
                }
            }
            kiss_out << ".end_kiss";
            kiss_out.close();
        }
    }
    else
        cout << "parameter input error" << endl;
}

int get_position(set<string> terms, string state)
{
    int index = 0;
    bool find = false;
    for (const auto &i : terms)
    {
        if (i == state)
        {
            find = true;
            break;
        }
        else
            index++;
    }
    if (find)
        return index;
    else
        return -1;
}

string get_input(set<string> terms, int pos)
{

    int count = 0;
    string str;
    for (const auto &i : terms)
    {
        if (count == pos)
        {
            str = i;
            break;
        }
        count++;
    }
    return str;
}

int find(vector<string> unique, string target)
{
    for (int i = 0; i < unique.size(); i++)
    {
        if (unique[i] == target)
            return i;
    }
    return -1;
}
