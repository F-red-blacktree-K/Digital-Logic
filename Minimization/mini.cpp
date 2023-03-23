#include <iostream>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>

using namespace std;

struct mintreminfo
{
    int group = -2; //分類(預設為-2)
    set<int> relation_minterm;
    string literal;     //-index literal(index product term)
    bool input = false; //該minterm是否為輸入
    bool merge = true;  //是否與其他minterm合併
};

//作為接下來化簡用的儲存容器
struct simplify_container
{
    int minterm = -1;
    int group = -1;          //分類(預設為-1)
    set<int> tmp_relation;   //-關聯項:標記一起合併的minterm有哪一些
    string simplify_literal; //-簡化過後的literal
};

struct _prime_implicant
{
    string record_minimal;
    set<int> tmp_relation;
    bool EPI = false;
};

void Decimal_to_Binary(int, mintreminfo *); // 2轉10

vector<_prime_implicant> Qunie_McCluskey(int, mintreminfo *, vector<simplify_container>, vector<int>, vector<string> &);

void Find_Essential_PI(int, mintreminfo *, vector<_prime_implicant>, vector<string> &, vector<int>);

void output(string, int, vector<string>);

void petrick(vector<vector<string>>, int, vector<set<string>> &, set<string> &, int);

int main(int argc, char *argv[])
{
    //讀檔案 a.out test1.pla 自取檔名.pla
    if (argc == 3) //參數個數
    {
        ifstream file(argv[1]);
        string output_filename = argv[2]; //輸出檔案名稱
        string fileline;
        string product_term;
        vector<int> literal_dont; //用來存literal是否為don't care
        int variables_num;
        int product_term_nums;
        string literal_state;

        if (!file)
            cout << "file not found" << endl;
        else
        {
            //建立表格
            mintreminfo *karnaugh;
            while (!file.eof())
            {
                file >> fileline;
                if (fileline == ".i")
                {
                    file >> variables_num;
                    karnaugh = new mintreminfo[300];
                    Decimal_to_Binary(variables_num, karnaugh); // literal given
                }
                else if (fileline == ".p")
                {
                    //輸入題目的literal數量
                    file >> product_term_nums;

                    //根據題目的literal去尋找mintreminfo中相符的literal，
                    //如果符合表示該minterm是為輸入
                    int table_field = pow(2, variables_num);
                    for (int i = 0; i < product_term_nums; i++)
                    {
                        file >> product_term;
                        file >> literal_state;

                        for (int j = 0; j < table_field; j++) //逐一檢查literal 8-> 1000
                        {
                            bool comparison = true;
                            //如果確定已為輸入，就跳過不做判斷，所以只挑尚未確定的
                            //若確定為輸入但是literal_state為'-'時要做判斷
                            if (karnaugh[j].input == false || literal_state == '-')
                            {
                                for (int k = 0; k < variables_num; k++)
                                {
                                    //如果不相等且該位元也不是don't care的話就表示literal不同
                                    if ((karnaugh[j].literal[k] != product_term[k]) && (product_term[k] != '-'))
                                    {
                                        comparison = false;
                                        break;
                                    }
                                }
                            }

                            if (comparison) //如果比對成功
                            {
                                karnaugh[j].input = true;
                                karnaugh[j].merge = false;
                            }
                            //如果比對成功且literal_state為-，紀錄其index，表示該項literal不算輸出
                            if (comparison && literal_state == '-')
                                literal_dont.push_back(j);
                        }
                    }

                    int number_of_one = 0;
                    //根據minterm為輸入來決定要不要分類
                    for (int i = 0; i < table_field; i++) // 16
                    {
                        if (karnaugh[i].input)
                        {
                            //計算1的數量
                            for (int j = 0; j < product_term_nums; j++) // 4
                            {
                                if (karnaugh[i].literal[j] == '1')
                                    number_of_one++;
                            }
                            karnaugh[i].group = number_of_one; // 將minterm分類
                        }
                        number_of_one = 0;
                    }
                }
            }

            vector<simplify_container> simplify_table; // 儲存簡化用結構
            vector<_prime_implicant> EPI;
            vector<string> final_EPI;
            EPI = Qunie_McCluskey(variables_num, karnaugh, simplify_table, literal_dont, final_EPI);

            Find_Essential_PI(variables_num, karnaugh, EPI, final_EPI, literal_dont);

            output(output_filename, variables_num, final_EPI);
        }
    }
    else
        cout << "parameter input error" << endl;
}

void Decimal_to_Binary(int variables_num, mintreminfo *karnaugh)
{
    int size = pow(2, variables_num);
    for (int i = 0; i < size; i++)
    {
        int temp = i;
        string bit;
        for (int j = 0; j < variables_num; j++)
        {
            if (temp % 2 == 0)
                bit.push_back('0');
            else if (temp % 2 == 1)
                bit.push_back('1');

            temp /= 2;
        }
        reverse(bit.begin(), bit.end());
        karnaugh[i].literal = bit;
        karnaugh[i].relation_minterm.insert(i);
    }
}

vector<_prime_implicant> Qunie_McCluskey(int variables_num, mintreminfo *karnaugh, vector<simplify_container> store, vector<int> literal_dont, vector<string> &final_EPI)
{
    vector<_prime_implicant> prime_implicant;

    int table_field = pow(2, variables_num);
    int bit_diff = 0;
    int record_bit = -1;
    bool simplify_flag = true;
    while (simplify_flag)
    {
        //簡化狀態一開始設定為false，表示已經沒有literal可以簡化了
        simplify_flag = false;
        for (int i = 0; i < table_field; i++) //遍歷整個table
        {
            for (int j = 0; j < table_field; j++) //尋找與分類差一的minterm
            {
                //化簡只能與相差一個位元的minterm進行
                if (karnaugh[i].group + 1 == karnaugh[j].group) //只與自己分類相差一的做比較
                {
                    bit_diff = 0;
                    record_bit = -1;
                    for (int k = 0; k < variables_num; k++)
                    {
                        //不同的位元且literal不為空
                        if ((karnaugh[i].literal[k] != karnaugh[j].literal[k]) && (karnaugh[j].literal != ""))
                        {
                            bit_diff++;
                            record_bit = k;
                            //紀錄不同位元的位置，如果有兩個位元不同，導致record_bit被刷掉也沒關係
                            //因為只有在一個不同位元的情況下才會合併
                        }
                    }

                    //如果位元相差不超過1個，找出不同位元的位置使其變成don't care(表示合併)
                    if (bit_diff == 1)
                    {
                        simplify_flag = true; //表示有literal簡化
                        string tmp_literal = karnaugh[i].literal;
                        tmp_literal[record_bit] = '-';
                        karnaugh[i].merge = true;
                        karnaugh[j].merge = true;

                        //將結果存到另一個vector中
                        simplify_container tmp;

                        for (const auto &j : karnaugh[i].relation_minterm)
                            tmp.tmp_relation.insert(j); //該minterm的關聯項

                        for (const auto &z : karnaugh[j].relation_minterm)
                            tmp.tmp_relation.insert(z); //該minterm的關聯項

                        tmp.simplify_literal = tmp_literal; //合併完的結果加入

                        tmp.group = karnaugh[i].group; // 分類狀況

                        store.push_back(tmp);
                    }
                }
            }
        }

        //該函式主軸，找出PI
        for (int i = 0; i < table_field; i++)
        {
            //檢查重複
            bool check = false;
            if (prime_implicant.size() != 0) //如果prime_implicant不為空才做重複判斷
            {
                for (int j = 0; j < prime_implicant.size(); j++)
                {
                    if (prime_implicant[j].record_minimal == karnaugh[i].literal)
                        check = true;
                }
            }

            // literal沒有與其他人合併且也不為空且PI中沒有重複項
            if (karnaugh[i].merge == false && karnaugh[i].literal != "" && check == false)
            {
                _prime_implicant pi_tmp;
                pi_tmp.record_minimal = karnaugh[i].literal;
                for (const auto &k : karnaugh[i].relation_minterm)
                    pi_tmp.tmp_relation.insert(k);

                prime_implicant.push_back(pi_tmp);
            }
        }

        //先將karnaugh的舊資料，literal、relation_minterm清除
        for (int i = 0; i < table_field; i++)
        {
            karnaugh[i].literal.clear();
            karnaugh[i].relation_minterm.clear();
            karnaugh[i].merge = false;
        }

        //將vector中的化簡結果存入karnaugh中
        for (int i = 0; i < store.size(); i++)
        {
            // relation_minterm
            for (const auto &j : store[i].tmp_relation)
                karnaugh[i].relation_minterm.insert(j);

            // literal
            int tmp_simplify_literal = store[i].simplify_literal.size();
            for (int k = 0; k < tmp_simplify_literal; k++)
                karnaugh[i].literal = store[i].simplify_literal;

            // group
            karnaugh[i].group = store[i].group;
        }

        //已經沒東西化簡，則將karnaugh中的東西加到EPI去
        if (!simplify_flag)
            break;

        //將vector中的資料清除
        vector<simplify_container>().swap(store);
    }
    return prime_implicant;
}

void Find_Essential_PI(int variables_num, mintreminfo *karnaugh, vector<_prime_implicant> simplify_literal, vector<string> &final_EPI, vector<int> literal_dont)
{
    // find EPI
    int arrsize = pow(2, variables_num);
    int count[arrsize] = {0};
    int simplify_literal_size = simplify_literal.size();

    for (int i = 0; i < simplify_literal_size; i++)
    {
        //統計關聯minterm數量
        for (const auto &j : simplify_literal[i].tmp_relation)
            count[j]++;
    }

    // don't care 的 literal不加入petrick's method中計算
    for (int i = 0; i < literal_dont.size(); i++)
        count[literal_dont[i]] = -1;

    for (int i = 0; i < arrsize; i++) //尋找統計陣列(同時i代表mintrem)
    {
        //找出只有被一個product term包住的mintrem
        if (count[i] == 1)
        {
            for (int j = 0; j < simplify_literal_size; j++) //發現只有1，去所有PI項尋找關聯項
            {
                for (const auto &k : simplify_literal[j].tmp_relation) //遍歷關聯項
                {
                    //如果找到只有被一個product term包住的mintrem
                    if (k == i)
                    {
                        final_EPI.push_back(simplify_literal[j].record_minimal); //此literal直接為EPI
                        simplify_literal[j].EPI = true;                          //等等petrick's method會選中false的literal

                        //如果找到該mintrem，順帶將其他的mintrem換成-1，等等petrick's method會選中除了-1以外的mintrem
                        for (const auto &z : simplify_literal[j].tmp_relation)
                            count[z] = -1;
                    }
                }
            }
        }
    }

    // petrick's method
    int nums = 0;
    vector<set<string>> sop;
    set<string> temp;

    //找出剩餘的minterm
    vector<int> remaining_mintern;
    for (int i = 0; i < arrsize; i++)
    {
        if (count[i] > 1)
            remaining_mintern.push_back(i);
    }
    int minterm_num = remaining_mintern.size();

    //如果EPI的數量為0，則不做petrick's method
    if (minterm_num != 0)
    {
        //建立pos
        vector<vector<string>> pos;
        for (int i = 0; i < minterm_num; i++)
        {
            vector<string> plus;
            for (int j = 0; j < simplify_literal.size(); j++)
            {
                for (const auto &z : simplify_literal[j].tmp_relation)
                    if (z == remaining_mintern[i])
                        plus.push_back(simplify_literal[j].record_minimal);
            }
            pos.push_back(plus);
        }

        //遞迴展開所有組合
        petrick(pos, nums, sop, temp, minterm_num);

        //尋找最小的組合
        int size = sop.size();
        int min = sop[0].size();
        int mini = 0;
        for (int i = 1; i < size; i++)
        {
            if (sop[i].size() < min)
            {
                min = sop[i].size();
                mini = i;
            }
        }

        for (const auto &i : sop[mini])
            final_EPI.push_back(i);
    }
}

void output(string outputname, int variables_num, vector<string> final_simplification)
{
    //輸出檔案
    fstream file_out(outputname, std::ios_base::out);

    file_out << ".i " << variables_num << endl;
    file_out << ".o " << 1 << endl;
    file_out << ".lib ";
    char letter = 97;
    for (int i = 0; i < variables_num; i++)
    {
        file_out << letter << " ";
        letter++;
    }
    file_out << endl;
    file_out << ".ob f" << endl;
    file_out << ".p " << final_simplification.size() << endl;

    for (int i = 0; i < final_simplification.size(); i++)
        file_out << final_simplification[i] << " 1" << endl;
    file_out << ".e" << endl;
    file_out.close();
}

void petrick(vector<vector<string>> pos, int nums, vector<set<string>> &sop, set<string> &temp, int size)
{
    if (nums < size - 1)
    {
        for (int i = 0; i < pos[nums].size(); i++)
        {
            if (temp.find(pos[nums][i]) == temp.end())
            {
                temp.insert(pos[nums][i]);
                petrick(pos, nums + 1, sop, temp, size);
                temp.erase(pos[nums][i]);
            }
            else
                petrick(pos, nums + 1, sop, temp, size);
        }
    }
    else if (nums == size - 1)
    {
        for (int i = 0; i < pos[nums].size(); i++)
        {
            if (temp.find(pos[nums][i]) == temp.end())
            {
                temp.insert(pos[nums][i]);
                sop.push_back(temp);
                temp.erase(pos[nums][i]);
            }
            else
                sop.push_back(temp);
        }
    }
}