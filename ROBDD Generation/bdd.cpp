#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace std;
//讀檔案
//建表
//根據規則化簡
//輸出
int main(int argc, char *argv[])
{
    //讀檔案
    if (argc == 2) //參數個數
    {
        ifstream file(argv[1]);

        if (!file)
            cout << "file not found" << endl;
        else
        {
            string fileline;
            string product_term;
            int variables_num = 0;
            int node_num = 0;
            int product_term_nums = 0;
            int pass = 0;
            int assign_value = 0;

            // table
            // char letter[128] = {'0'};
            // char ilb[128] = {'0'};
            // char state[128] = {'0'}; // r、n、0
            // int then_edge[128] = {0};
            // int else_edge[128] = {0};

            char *state;
            char *letter;
            char *ilb;
            int *then_edge;
            int *else_edge;

            while (!file.eof())
            {
                //建立表格
                file >> fileline;
                if (fileline == ".i")
                {
                    file >> variables_num;

                    node_num = pow(2, variables_num) - 1;
                    assign_value = pow(2, variables_num - 1) - 1;

                    state = new char[node_num + 1];
                    letter = new char[variables_num];
                    ilb = new char[node_num + 1];
                    then_edge = new int[node_num + 1];
                    else_edge = new int[node_num + 1];

                    //給值跑到pow(2, 變數數量-1) - 1  然後node也等於index為pow2(2, 變數數量) - 1
                    for (int index = 1; index <= assign_value; index++)
                    {
                        then_edge[index] = 2 * index + 1;
                        else_edge[index] = 2 * index;
                    }

                    for (int index = 1; index <= node_num; index++)
                        state[index] = 'n'; // n表示非redundent狀態

                    state[0] = '0';            // Boolean0
                    state[node_num + 1] = '1'; // Boolean1
                }
                else if (fileline == ".ilb")
                {
                    for (int i = 1; i <= variables_num; i++)
                        file >> letter[i];

                    int power = 1;
                    for (int index = 1; index <= node_num; index++)
                    {
                        ilb[index] = letter[power];
                        if (index == pow(2, power) - 1) //到最右邊要換字母
                            power++;
                    }

                    ilb[0] = '0';
                    ilb[node_num + 1] = '1';
                }
                else if (fileline == ".p")
                {
                    file >> product_term_nums;
                    vector<string> bit_table;
                    //求出變數的所有組合
                    for (int j = 0; j < node_num + 1; j++) // j:0~7
                    {
                        string bit;
                        int calculate = j;
                        for (int k = 0; k < variables_num; k++) //幾變數除幾次
                        {
                            if (calculate % 2 == 0)
                                bit.push_back('0');
                            else if (calculate % 2 == 1)
                                bit.push_back('1');

                            calculate /= 2;
                        }
                        reverse(bit.begin(), bit.end());
                        bit_table.push_back(bit); //將結果加入vector中
                    }

                    for (int i = 0; i < product_term_nums; i++)
                    {
                        file >> product_term;

                        // product termm與string vector比對
                        int table_index = pow(2, variables_num - 1);
                        for (int j = 0; j < bit_table.size(); j++)
                        {
                            if (j > 0 && j % 2 == 0)
                                table_index++;

                            bool compare = true;
                            for (int k = 0; k < variables_num; k++) //逐字比對
                            {
                                //一旦發現位元不一樣且該格也不是'-'
                                if (bit_table[j][k] != product_term[k] && product_term[k] != '-')
                                {
                                    //位元只要對不上就直接跳過
                                    compare = false;
                                    break;
                                }
                            }
                            //如果都一樣compare沒有變成fasle 表示比對成功，則該格給1
                            if (compare)
                            {
                                // then else 切換
                                if (j % 2 == 0)
                                    else_edge[table_index] = node_num + 1; // 8
                                else if (j % 2 == 1)
                                    then_edge[table_index] = node_num + 1; // 8
                            }
                        }
                        file >> pass;
                    }
                }
            }

            bool check = true;
            while (check)
            {
                check = false;
                //根據規則化簡
                //規則1:檢查是否有多個節點的then_edge與else_edge是一樣的
                for (int i = 1; i <= node_num; i++)
                {
                    for (int j = i + 1; j <= node_num; j++)
                    {
                        //節點的then_edge與else_dege一樣且不為redundent
                        if (else_edge[i] == else_edge[j] && then_edge[i] == then_edge[j] && state[j] == 'n' && ilb[i] == ilb[j])
                        {
                            check = true;
                            state[j] = 'r';
                            for (int k = 1; k <= node_num; k++)
                            {
                                if (else_edge[k] == j)
                                    else_edge[k] = i;
                                if (then_edge[k] == j)
                                    then_edge[k] = i;
                            }
                        }
                    }
                }

                //規則2:檢查節點的then_edge與else_edge是否一樣
                for (int index = 1; index <= node_num; index++)
                {
                    int tmp_index = 0;
                    int save_edge = -1;
                    // 如果一樣且該點還不是redundent，則該節點標為redundent
                    if (then_edge[index] == else_edge[index] && state[index] == 'n')
                    {
                        state[index] = 'r';
                        save_edge = then_edge[index];
                        tmp_index = index;
                        //將連接到該redundent節點的edge改成redundent節點的then_edge
                        for (int i = 1; i <= node_num; i++)
                        {
                            check = true;
                            if (then_edge[i] == tmp_index)
                                then_edge[i] = save_edge;
                            else if (else_edge[i] == tmp_index)
                                else_edge[i] = save_edge;
                        }
                    }
                }
            }

            //將化簡結果輸出成dot檔案，並且使用graphviz繪製圖形

            fstream file_out;
            file_out.open("robdd.dot", ios::out); //開檔案

            file_out << "digraph "
                     << "robdd"
                     << " {" << endl;

            vector<int> not_redundent;

            //將table中不是redundent的節點將其加入not_redundent的vector
            for (int index = 1; index <= node_num; index++)
            {
                if (state[index] == 'n')
                    not_redundent.push_back(index);
            }

            //建立rank
            for (int i = 1; i <= variables_num; i++) // level
            {
                string rank = "   {rank=same";
                for (int j = 0; j < not_redundent.size(); j++)
                {
                    if (letter[i] == ilb[not_redundent[j]])
                    {
                        rank += " ";
                        rank += to_string(not_redundent[j]);
                    }
                }
                file_out << rank << "}" << endl;
            }

            //建立label
            file_out << "	0 [label=0, shape=box]" << endl;

            //去not_redundent陣列中取出對應的字母
            for (int i = 0; i < not_redundent.size(); i++)
                file_out << "	" << not_redundent[i] << " [label=\"" << ilb[not_redundent[i]] << "\"]" << endl;

            file_out << "   " << node_num + 1 << " [label=1, shape=box]" << endl;

            //節點之間的關係
            for (int i = 0; i < not_redundent.size(); i++)
            {
                file_out << "   " << not_redundent[i] << " -> " << else_edge[not_redundent[i]] << " [label=\"0\", style=dotted]" << endl;
                file_out << "   " << not_redundent[i] << " -> " << then_edge[not_redundent[i]] << " [label=\"1\", style=solid]" << endl;
            }
            file_out << "}";
            file_out.close();

            delete[] state;
            delete[] letter;
            delete[] ilb;
            delete[] then_edge;
            delete[] else_edge;
        }
    }
    else
        cout << "parameter input error" << endl;
}
