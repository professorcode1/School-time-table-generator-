#include <bits/stdc++.h>
#include <future>
#include <thread>
using namespace std;
int number_of_periods_per_day, number_of_days_school_is_open, CLASSES;
bool first_period_must_be_class_teacher;
const int population_size{800}, convergence_criteria{40};
const float mutation_intensity{0.01}, fraction_of_population_mutated{0.25}, fraction_of_population_elite{0.1}, mutation_increase_factor{100};
mt19937 g2(chrono::system_clock::now().time_since_epoch().count());

int sudorandom_number_generator(int left, int right)
{
    long long l = static_cast<long long>(left), r = static_cast<long long>(right);
    long long length = r - l + 1;
    long long z = (g2() % length) + l;
    if (z > right || z < left)
    {
        cout << "error, random number generator not working.\n";
        cin.get();
    }
    return z;
}
class teacher;
class classroom;
class time_table;
class subject
{
private:
    string subject_name;
    int subject_id;
    static int subject_id_gnrtr;
    vector<list<teacher>::iterator> taught_by;
    vector<list<classroom>::iterator> taught_in;

public:
    subject(string subject_name)
    {
        this->subject_name = subject_name;
        subject_id = subject_id_gnrtr++;
    }
    string getter_subject_name()
    {
	    return this->subject_name;
    }
    void append_taught_by(list<teacher>::iterator new_teacher)
    {
        taught_by.push_back(new_teacher);
    }
    void append_taught_in(list<classroom>::iterator new_classroom)
    {
        taught_in.push_back(new_classroom);
    }
    friend ostream &operator<<(ostream &out, const subject &object);
    friend ostream &operator<<(ostream &out, const classroom &object);
    friend ostream &operator<<(ostream &out, const teacher &object);
    friend ostream &operator<<(ostream &out, const time_table &object);
    friend class time_table;
};
int subject::subject_id_gnrtr{0};
class classroom
{
private:
    int classroom_id, class_number;
    static int classroom_id_gnrtr;
    char class_section;
    list<teacher>::iterator homeroom_teacher;
    vector<pair<list<subject>::iterator, int>> subjects_and_hours_per_week;

public:
    classroom(int class_number, char class_section, list<teacher>::iterator homeroom_teacher)
    {
        this->class_number = class_number;
        this->class_section = class_section;
        this->homeroom_teacher = homeroom_teacher;
        classroom_id = classroom_id_gnrtr++;
    }
    void append_subjects_and_hours_per_week(pair<list<subject>::iterator, int> new_subject_and_hours_couple)
    {
        subjects_and_hours_per_week.push_back(new_subject_and_hours_couple);
    }
    int getter_class_number()
    {
        return class_number;
    }
    char getter_class_section()
    {
        return class_section;
    }
    int hours_this_subject(list<subject>::iterator subject_being_check)
    {
        for (int i = 0; i < subjects_and_hours_per_week.size(); i++)
            if (subjects_and_hours_per_week.at(i).first == subject_being_check)
                return subjects_and_hours_per_week.at(i).second;
        return 0;
    }
    friend ostream &operator<<(ostream &out, const subject &object);
    friend ostream &operator<<(ostream &out, const classroom &object);
    friend ostream &operator<<(ostream &out, const teacher &object);
    friend ostream &operator<<(ostream &out, const time_table &object);
    friend class time_table;
};
int classroom::classroom_id_gnrtr{0};
class teacher
{
private:
    string teacher_name;
    int teacher_id;
    static int teacher_id_gnrtr;
    list<subject>::iterator teaches;
    list<classroom>::iterator homeroom_of;
    bool is_homeroom; //is homeroom of some class
    vector<list<classroom>::iterator> class_preference;

public:
    teacher(string teacher_name, list<subject>::iterator teaches)
    {
        this->teacher_name = teacher_name;
        teacher_id = teacher_id_gnrtr++;
        this->teaches = teaches;
        is_homeroom = false;
    }
    string getter_teacher_name()
    {
        return teacher_name;
    }
    void is_homeroom_teacher(list<classroom>::iterator homeroom_of)
    {
        this->homeroom_of = homeroom_of;
        is_homeroom = true;
    }
    void append_class_preference(list<classroom>::iterator new_classroom)
    {
        class_preference.push_back(new_classroom);
    }
    void print_class_preference()
    {
        for (int i = 0; i < class_preference.size(); i++)
            cout << i << ") " << class_preference.at(i)->getter_class_number() << class_preference.at(i)->getter_class_section() << " ";
    }
    friend ostream &operator<<(ostream &out, const subject &object);
    friend ostream &operator<<(ostream &out, const classroom &object);
    friend ostream &operator<<(ostream &out, const teacher &object);
    friend ostream &operator<<(ostream &out, const time_table &object);
    friend class time_table;
};
int teacher::teacher_id_gnrtr{0};
class time_table
{
private:
    vector<vector<vector<list<teacher>::iterator>>> table;
    list<classroom>::iterator classroom_iterator_begin;
    list<teacher>::iterator teachers_iterator_end;
    static int teacher_hlpr_heuristic;

public:
    float fitness_value;
    time_table(const time_table &original)
    {
        table.resize(CLASSES);

        for (int i = 0; i < CLASSES; i++)
            table[i].resize(number_of_days_school_is_open);

        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                table[i][j].resize(number_of_periods_per_day);

        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    table[i][j][k] = original.table[i][j][k];

        this->classroom_iterator_begin = original.classroom_iterator_begin;
        this->teachers_iterator_end = original.teachers_iterator_end;
        fitness_value = this->calc_fitness();
    }
    time_table(const time_table &original1, const time_table &original2)
    {
        table.resize(CLASSES);

        for (int i = 0; i < CLASSES; i++)
            table[i].resize(number_of_days_school_is_open);

        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                table[i][j].resize(number_of_periods_per_day);

        for (int i = 0; i < CLASSES; i++)
        {
            if (sudorandom_number_generator(0, 999) <= 500)
                for (int j = 0; j < number_of_days_school_is_open; j++)
                    for (int k = 0; k < number_of_periods_per_day; k++)
                        table[i][j][k] = original1.table[i][j][k];
            else
            {
                for (int j = 0; j < number_of_days_school_is_open; j++)
                    for (int k = 0; k < number_of_periods_per_day; k++)
                        table[i][j][k] = original2.table[i][j][k];
            }
        }
        this->classroom_iterator_begin = original1.classroom_iterator_begin;
        this->teachers_iterator_end = original1.teachers_iterator_end;
        fitness_value = this->calc_fitness();
    }
    time_table(list<subject> &subjects, list<teacher> &teachers, list<classroom> &classrooms)
    {
        //first checking if the time table if possible
        {
            bool possible{true};
            for (list<subject>::iterator subjects_itrt = subjects.begin(); subjects_itrt != subjects.end(); subjects_itrt++)
            {
                int hours{0};
                for (const auto hlpr_vctr_psr : subjects_itrt->taught_in)
                    hours += hlpr_vctr_psr->hours_this_subject(subjects_itrt);
                if (hours > subjects_itrt->taught_by.size() * number_of_periods_per_day * number_of_days_school_is_open)
                {
                    possible = false;
                    cout << "The subject " << subjects_itrt->subject_name << " has so few teachers it is mathematically impossible to make the time table.\n";
                    cout << "To use this program, enter a hypothetical teacher in the list of teacher who teach this subject.\n";
                    cout << "This way the time table will become mathematically possible.\n";
                    cout << "You can proced with the program but you will experience Uncoumented Behaviour(crash/segmentation fault).\n";
                    cout << "So just close and restart.\n";
                    break;
                }
            }
        }
        cout << "In the next section for each subject, you have to assign the classes it is taught in, to the teachers who teach them.\n\n\n";
        for (list<subject>::iterator subjects_itrt = subjects.begin(); subjects_itrt != subjects.end(); subjects_itrt++)
        {
            vector<list<classroom>::iterator> taught_in(subjects_itrt->taught_in);
            vector<list<teacher>::iterator> taught_by(subjects_itrt->taught_by);
            vector<int> hours_per_teacher(taught_by.size(), 0);
            for (int i = 0; i < taught_in.size(); i++)
                if (taught_in.at(i)->homeroom_teacher->teaches == subjects_itrt)
                {
                    taught_in.erase(taught_in.begin() + i);
                    i--;
                }
            cout << "At any point you can perform 3 quiery by entering one of 3 integers\n";
            cout << "Entering 1 will move you on to the next subject. If there are classes left unassigned then the program will equaly distribute them among the teachers.\n";
            cout << "Entering 2 will allow you to pick a class from the class list and give it to a teacher\n";
            cout << "Entering 3 will allow you to pick a class from a teachers list and assign it to a different teacher.\n";
            cout << "The subject " << subjects_itrt->subject_name << " is being worked at.\n";
            while (true)
            {
                cout << "The current state.\n";
                if (not taught_in.empty())
                {
                    cout << "The following classes are unassigned.\n";
                    for (int i = 0; i < taught_in.size(); i++)
                        cout << i << ") " << taught_in.at(i)->class_number << taught_in.at(i)->class_section << "  ";
                    cout << endl;
                }
                else
                {
                    cout << "All classes have been assigned.If you are ok with the distribution and don't wanna make any changes press 1 to continue.\n";
                    cout << "Else press 3 to make changes.\n";
                }
                for (int i = 0; i < taught_by.size(); i++)
                {
                    cout << i << ")" << taught_by.at(i)->teacher_name << " -> ";
                    taught_by.at(i)->print_class_preference();
                    cout << endl;
                }
                int user_choice;
                cin >> user_choice;
                if (user_choice == 1)
                {
                    while (not taught_in.empty())
                    {
                        int index = distance(hours_per_teacher.begin(), min_element(hours_per_teacher.begin(), hours_per_teacher.end()));
                        taught_by.at(index)->class_preference.push_back(taught_in.back());
                        hours_per_teacher.at(index) += taught_in.back()->hours_this_subject(subjects_itrt);
                        taught_in.pop_back();
                    }
                    cout<<"\n\n\n";
                    break;
                }
                if (user_choice == 2)
                {
                    bool doable{true};
                    cout << "Enter the number assosiated with the class you want to pick up and enter the number assosiated with the teacher you want to dump it to.\n";
                    int num1, num2;
                    cin >> num1 >> num2;
                    if (num1 < 0 || num1 >= taught_in.size())
                    {
                        cout << "The first number is not associated to any class.\n";
                        doable = false;
                    }
                    if (doable)
                        if (num2 < 0 || num2 >= taught_by.size())
                        {
                            cout << "The second number is not assosiated to any teacher.\n";
                            doable = false;
                        }
                    if (doable)
                        if (hours_per_teacher.at(num2) + taught_in.at(num1)->hours_this_subject(subjects_itrt) > number_of_periods_per_day * number_of_days_school_is_open)
                        {
                            cout << "This transfer cannot be performed as the number of hours taught by" << taught_by.at(num2)->teacher_name << " teaches will become ";
                            cout << hours_per_teacher.at(num2) + taught_in.at(num1)->hours_this_subject(taught_by.at(num2)->teaches) << endl;
                            cout << "And that makes the time table impossible.\n";
                            cout << "As the maximum periods a teacher can have are " << number_of_days_school_is_open * number_of_periods_per_day;
                            doable = false;
                        }
                    if (doable)
                    {
                        taught_by.at(num2)->class_preference.push_back(taught_in.at(num1));
                        hours_per_teacher.at(num2) += taught_in.at(num1)->hours_this_subject(subjects_itrt);
                        taught_in.erase(taught_in.begin() + num1);
                    }
                }
                if (user_choice == 3)
                {
                    bool doable{true};
                    cout << "Enter the number associated with the teacher from whom you want to pick a class.\n";
                    cout << "Then enter the number associated with the class from their list.\n";
                    cout << "Then enter the numbe associated with the teacher you want to drop the class to.\n";
                    int num1, num2, num3;
                    cin >> num1 >> num2 >> num3;
                    if (num1 < 0 || num1 >= taught_by.size())
                    {
                        cout << "The first number is not assosiated to any teacher.\n";
                        doable = false;
                    }
                    if (doable)
                        if (num2 < 0 || num2 >= taught_by.at(num1)->class_preference.size())
                        {
                            cout << "The second number is not assosiated to any class taught by " << taught_by.at(num1)->teacher_name << '\n';
                            doable = false;
                        }
                    if (doable)
                        if (num3 < 0 || num3 >= taught_by.size())
                        {
                            cout << "The third number is not assosiated to any teacher.\n";
                            doable = false;
                        }
                    if (doable)
                        if (first_period_must_be_class_teacher)
                            if (taught_by.at(num1)->class_preference.at(num2)->homeroom_teacher == taught_by.at(num1))
                            {
                                cout << "The class " << taught_by.at(num1)->class_preference.at(num2)->class_number;
                                cout << taught_by.at(num1)->class_preference.at(num2)->class_section << " has ";
                                cout << taught_by.at(num1)->teacher_name << " as homeroom.\n So class shift cannot be performed";
                                doable = false;
                            }
                    if (doable)
                        if (hours_per_teacher.at(num3) + taught_by.at(num1)->class_preference.at(num2)->hours_this_subject(subjects_itrt) > number_of_days_school_is_open * number_of_periods_per_day)
                        {
                            cout << "This transfer cannot be performed as the number of hours " << taught_by.at(num3)->teacher_name << " teaches will become ";
                            cout << hours_per_teacher.at(num3) + taught_by.at(num1)->class_preference.at(num2)->hours_this_subject(subjects_itrt) << endl;
                            cout << "And that makes the time table impossible.\n";
                            cout << "As the maximum periods a teacher can have are " << number_of_days_school_is_open * number_of_periods_per_day;
                            doable = false;
                        }
                    if (doable)
                    {
                        taught_by.at(num3)->class_preference.push_back(taught_by.at(num1)->class_preference.at(num2));
                        hours_per_teacher.at(num1) -= taught_by.at(num1)->class_preference.at(num2)->hours_this_subject(subjects_itrt);
                        hours_per_teacher.at(num3) += taught_by.at(num1)->class_preference.at(num2)->hours_this_subject(subjects_itrt);
                        taught_by.at(num1)->class_preference.erase(find(taught_by.at(num1)->class_preference.begin(), taught_by.at(num1)->class_preference.end(), taught_by.at(num1)->class_preference.at(num2)));
                    }
                }
                cout << "\n\n\n";
            }
        }
        table.resize(CLASSES);

        for (int i = 0; i < CLASSES; i++)
            table[i].resize(number_of_days_school_is_open);

        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                table[i][j].resize(number_of_periods_per_day);

        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    table[i][j][k] = teachers.end();

        {
            int continue_index_arr[CLASSES] = {0}, hours;
            if (first_period_must_be_class_teacher)
            {
                fill_n(continue_index_arr, CLASSES, number_of_days_school_is_open);
                for (list<classroom>::iterator classroom_itrt = classrooms.begin(); classroom_itrt != classrooms.end(); classroom_itrt++)
                {
                    int index = distance(classrooms.begin(), classroom_itrt);
                    for (int i = 0; i < number_of_days_school_is_open; i++)
                        table[index][i][0] = classroom_itrt->homeroom_teacher;
                }
            }
            for (list<teacher>::iterator teachers_itrt = teachers.begin(); teachers_itrt != teachers.end(); teachers_itrt++)
                for (list<classroom>::iterator teacher_class_prefernce_passer : teachers_itrt->class_preference)
                {
                    hours = teacher_class_prefernce_passer->hours_this_subject(teachers_itrt->teaches);
                    if (hours >= number_of_periods_per_day)
                        time_table::teacher_hlpr_heuristic++;
                    if (first_period_must_be_class_teacher && teacher_class_prefernce_passer->homeroom_teacher == teachers_itrt)
                        hours -= number_of_days_school_is_open;
                    while (hours--)
                    {
                        int index = distance(classrooms.begin(), teacher_class_prefernce_passer);
                        table[index][continue_index_arr[index] % number_of_days_school_is_open][continue_index_arr[index] / number_of_days_school_is_open] = teachers_itrt;
                        continue_index_arr[index]++;
                    }
                }
        }
        classroom_iterator_begin = classrooms.begin();
        teachers_iterator_end = teachers.end();
        fitness_value = this->calc_fitness();
    }
    void shuffle()
    {
        // for (int i = 0; i < CLASSES; i++)
        // {
        //     for (int ij = first_period_must_be_class_teacher ? number_of_days_school_is_open : 0; ij < number_of_days_school_is_open * number_of_periods_per_day; ij++)
        //     {
        //         int rndm_indx = sudorandom_number_generator(ij, (number_of_days_school_is_open * number_of_periods_per_day) - 1);
        //         swap(table[i][ij % number_of_days_school_is_open][ij / number_of_days_school_is_open], table[i][rndm_indx % number_of_days_school_is_open][rndm_indx / number_of_days_school_is_open]);
        //     }
        // }
        // fitness_value = this->calc_fitness();
        for (int i = 0; i < CLASSES; i++)
        {
            for (int j = first_period_must_be_class_teacher ? 1 : 0; j < number_of_periods_per_day; j++)
            {
                int rndm = sudorandom_number_generator(j, number_of_periods_per_day - 1);
                for (int k = 0; k < number_of_days_school_is_open; k++)
                    swap(table[i][k][j], table[i][k][rndm]);
            }
        }
        fitness_value = this->calc_fitness();
    }
    int clashes()
    {
        //It looks bad but its the oprtimal code for calculating clashes
        int clashes = 0;
        bool accounted_for;
        for (int j = 0; j < number_of_days_school_is_open; j++)
        {
            for (int k = 0; k < number_of_periods_per_day; k++)
            {
                for (int i = 0; i < CLASSES; i++)
                {
                    accounted_for = true;
                    for (int x = i - 1; x >= 0; x--)
                        accounted_for = accounted_for && table[i][j][k] != table[x][j][k];
                    bool occurs_again = false;
                    if (accounted_for)
                        for (int x = i + 1; x < CLASSES; x++)
                            if (table[i][j][k] == table[x][j][k])
                            {
                                clashes++;
                                occurs_again = true;
                            }
                    if (occurs_again)
                        clashes++;
                }
            }
        }
        return clashes;
    }
    float calc_fitness()
    {
        int students_get_different_periods_per_day{0}, teacher_teach_same_class_different_days{0};
        float ans{0};

        //measures => how many distinct teachers take the same period in a class on different days.
        //decreasing this promotes a time table symmetric for teacher
        int arr[number_of_days_school_is_open];
        for (int i = 0; i < CLASSES; i++)
        {
            for (int k = 0; k < number_of_periods_per_day; k++)
            {
                for (int j = 0; j < number_of_days_school_is_open; j++)
                    arr[j] = table[i][j][k]->teacher_id;
                sort(arr, arr + number_of_days_school_is_open);
                int freq = 1;
                for (int j = 1; j < number_of_days_school_is_open; j++)
                    if (arr[j] != arr[j - 1])
                        freq++;
                if (freq == 1)
                    teacher_teach_same_class_different_days++;
            }
        }

        //measure => how many times the subject flips each day for students
        //increasing this makes it so that students don't get the same periods consecutively
        for (int i = 0; i < CLASSES; i++)
        {
            for (int j = 0; j < number_of_days_school_is_open; j++)
            {
                int freq = 1;
                for (int k = 1; k < number_of_periods_per_day; k++)
                    if (table[i][j][k] != table[i][j][k - 1])
                    {
                        if (freq == 1)
                            students_get_different_periods_per_day++;
                        freq = 1;
                    }
                    else
                        freq++;
                if (freq == 1)
                    students_get_different_periods_per_day++;
            }
        }
        ans += static_cast<float>(-100 * this->clashes()) + (float(teacher_hlpr_heuristic * students_get_different_periods_per_day) / float(10 * teacher_teach_same_class_different_days * CLASSES * number_of_days_school_is_open));
        return ans;
    }
    float fitness()
    {
        return fitness_value;
    }
    void mutate()
    {
        int first_index, second_index;
        bool first_conflicted{false}, second_conflicted{false};
        for (int i = 0; i < CLASSES; i++)
            if (sudorandom_number_generator(0, 999) >= 900)
                for (int j = 0; j < number_of_days_school_is_open; j++)
                    for (int k = first_period_must_be_class_teacher ? 1 : 0; k < number_of_periods_per_day; k++)
                    {
                        first_index = number_of_days_school_is_open * j + k;
                        second_index = sudorandom_number_generator(first_index, number_of_days_school_is_open * number_of_periods_per_day - 1);
                        first_conflicted = false;
                        second_conflicted = false;
                        for (int x = 0; x < CLASSES; x++)
                            if (x == i)
                                continue;
                            else
                                first_conflicted = first_conflicted || table[x][j][k] == table[i][j][k];
                        for (int x = 0; x < CLASSES; x++)
                            if (x == i)
                                continue;
                            else
                                second_conflicted = second_conflicted || table[x][second_index % number_of_days_school_is_open][second_index / number_of_days_school_is_open] == table[i][second_index % number_of_days_school_is_open][second_index / number_of_days_school_is_open];
                        float mutation_number = mutation_intensity;
                        if (first_conflicted || second_conflicted)
                            mutation_number *= mutation_increase_factor;
                        if (first_period_must_be_class_teacher && second_index % number_of_days_school_is_open == 0)
                            continue;
                        if (sudorandom_number_generator(0, 9999) <= ceil(10000 * mutation_number))
                        {
                            swap(table[i][j][k], table[i][second_index % number_of_days_school_is_open][second_index / number_of_days_school_is_open]);
                            return;
                        }
                    }
        fitness_value = this->calc_fitness();
    }
    friend bool operator<(time_table &object1, time_table &object2);
    friend ostream &operator<<(ostream &out, const time_table &object);
};
int time_table::teacher_hlpr_heuristic{0};
ostream &operator<<(ostream &out, const subject &object)
{
    out << "Subject name is " << object.subject_name << ".\nIts id is " << object.subject_id << ".\nIt is taught by \n";
    for (int i = 0; i < (object.taught_by).size(); i++)
        out << object.taught_by[i]->teacher_name << "\n";
    out << "\nIt is taught in the classes \n";
    for (int i = 0; i < object.taught_in.size(); i++)
        out << (*object.taught_in[i]).class_number << " " << object.taught_in[i]->class_section << " ";
    out << "\n\n";
    return out;
}
ostream &operator<<(ostream &out, const classroom &object)
{
    out << "Class is " << object.class_number << " " << object.class_section << ".\nIts ID is " << object.classroom_id << ".\n";
    out << "The homeroom teacher is " << object.homeroom_teacher->teacher_name << ".\n";
    out << "The subjects taught per hour are \n";
    for (int i = 0; i < object.subjects_and_hours_per_week.size(); i++)
        out << "Name:" << object.subjects_and_hours_per_week[i].first->subject_name << " Hours:" << object.subjects_and_hours_per_week[i].second << "\n";
    out << "\n";
    return out;
}
ostream &operator<<(ostream &out, const teacher &object)
{
    out << "Teacher name is " << object.teacher_name << ".\nTheir ID is " << object.teacher_id << ".\n";
    out << "The subject they teach is " << object.teaches->subject_name << endl;
    if (object.is_homeroom)
        out << "They are the homeroom of " << object.homeroom_of->class_number << " " << object.homeroom_of->class_section << '\n';
    out << "The classes they teach are :";
    for (int i = 0; i < object.class_preference.size(); i++)
        out << object.class_preference[i]->class_number << object.class_preference[i]->class_section << " ";
    out << "\n\n";
    return out;
}
ostream &operator<<(ostream &out, const time_table &object)
{
    list<classroom>::iterator rndm_it0 = object.classroom_iterator_begin;
    for (int i = 0; i < CLASSES; i++)
    {
        out << rndm_it0->class_number << " " << rndm_it0->class_section << endl;
        for (int j = 0; j < number_of_days_school_is_open; j++)
        {
            switch (j + 1)
            {
            case 1:
                out << "Monday         ";
                break;
            case 2:
                out << "Tuesday        ";
                break;
            case 3:
                out << "Wedesday       ";
                break;
            case 4:
                out << "Thursday       ";
                break;
            case 5:
                out << "Friday         ";
                break;
            case 6:
                out << "Saturday       ";
                break;
            }
            for (int k = 0; k < number_of_periods_per_day; k++)
            {
                if (object.table[i][j][k] == object.teachers_iterator_end)
                    out << " Free Period   ";
                else
                {
                    out << " ";
                    string str = object.table[i][j][k]->teacher_name;
                    int l = 0;
                    for (l = 0; str[l]; l++)
                    {
                        if (l == 14)
                            break;
                        out << str[l];
                    }
                    while (l < 15)
                    {
                        out << " ";
                        l++;
                    }
                }
            }
            out << '\n';
        }
        out << '\n';
        rndm_it0++;
    }
    return out;
}
bool operator<(time_table &object1, time_table &object2)
{
    return object1.fitness() > object2.fitness();
}
void merge(vector<time_table> &arr, int p, int q, int r)
{
    vector<time_table> arr1, arr2;
    for (int i = 0; i < q - p + 1; i++)
        arr1.push_back(arr[p + i]);

    for (int i = 0; i < r - q; i++)
        arr2.push_back(arr[q + 1 + i]);
    int x{0}, y{0}, i{0};
    for (i = p; i <= r; i++)
    {
        if (x == q - p + 1 || y == r - q)
            break;
        else if (arr1[x] < arr2[y])
            arr[i] = arr1[x++];
        else
            arr[i] = arr2[y++];
    }
    while (x < q - p + 1)
        arr[i++] = arr1[x++];
    while (y < r - q)
        arr[i++] = arr2[y++];
}
void merge_sort(vector<time_table> &arr, int l, int r)
{
    if (l == r)
        return;
    int p = l + (r - l) / 2;
    merge_sort(arr, l, p);
    merge_sort(arr, p + 1, r);
    merge(arr, l, p, r);
}
int tournament_selection(int left, int right)
{
    //the propability of a number returning is an AP. The number left has 'a' probability, left + 1 has 'a-d',left +2 has probability 'a-2d' and so on.right have prob 1/(right-left+1)
    //the mathematic formula for such a problem can be derived to what is shown below.
    int n = right - left + 1;
    while (true)
    {
        int rndm1 = sudorandom_number_generator(0, n - 1), rndm2 = sudorandom_number_generator(0, n - 1);
        if (rndm2 <= n - rndm1 - 1)
            return rndm1 + left;
    }
}
void genetic_algorithm_for_time_table(time_table initial_time_table)
{
    vector<time_table> previous_generation, next_generation(population_size, initial_time_table);
    for (int i = 0; i < population_size; i++)
        next_generation.at(i).shuffle();
    merge_sort(next_generation, 0, next_generation.size() - 1);
    int generation{0}, convergence_generation{0} , numberOfThreads = std::thread::hardware_concurrency(),crossovers = population_size-ceil(fraction_of_population_elite * population_size);
    cout<<crossovers<<endl;
    if(!numberOfThreads)
        numberOfThreads = 1; //might be zero if hardware_concurrency() fails
    const int workPerThread = floor(static_cast<float>(crossovers) / numberOfThreads);
    vector< future < vector < time_table > > > multiThread; //we multithread the crossover
    float convergence_value{next_generation.front().fitness()};
    while (convergence_value < 0 || generation - convergence_generation <= convergence_criteria)
    {
        previous_generation.clear();
        for (int i = 0; i < population_size; i++)
            previous_generation.push_back(next_generation.at(i));
        next_generation.clear();
        for (int i = 0; i < ceil(fraction_of_population_elite * population_size); i++)
            next_generation.push_back(previous_generation.at(i));

        //creating the rest of the population via selection and mixing
        //for (int loop_var = ceil(fraction_of_population_elite * population_size); loop_var < population_size; loop_var++)
        //    next_generation.push_back(time_table(previous_generation.at(tournament_selection(0, population_size - 1)), previous_generation.at(tournament_selection(0, population_size - 1))));

        multiThread.clear();
        for(int i=0 ; i<numberOfThreads-1 ; i++ )
            multiThread.push_back(async(std::launch::async,[&previous_generation](const int numberOfTablesRequired)->vector<time_table>{
                //cout<<numberOfTablesRequired << endl;
                vector<time_table> returnVec;
                for (int loop_var = 0; loop_var < numberOfTablesRequired; loop_var++)
                    returnVec.push_back(time_table(previous_generation.at(tournament_selection(0, population_size - 1)), previous_generation.at(tournament_selection(0, population_size - 1))));
                return returnVec;
            },(  workPerThread )));

            
        for (int loop_var = 0; loop_var < crossovers - (numberOfThreads-1) * workPerThread; loop_var++)
                    next_generation.push_back(time_table(previous_generation.at(tournament_selection(0, population_size - 1)), previous_generation.at(tournament_selection(0, population_size - 1))));
        
        for(int i=0 ; i<numberOfThreads-1 ; i++)
            for(const auto table : multiThread.at(i).get())
                next_generation.push_back(table);

        //mutating some of it
        for (int i = 0; i < ceil(fraction_of_population_mutated * population_size); i++)
            next_generation.at(sudorandom_number_generator(0, population_size - 1)).mutate();

        merge_sort(next_generation, 0, next_generation.size() - 1); //normal sorting causes bad alloc

        if (floor(next_generation.front().fitness() / 100) != convergence_value)
        {
            convergence_value = floor(next_generation.front().fitness() / 100);
            convergence_generation = generation;
        }
        if (next_generation.front().fitness() < 0)
            cout << "Currunt Generation ->" << ++generation << " and the sickness is " << -1 * next_generation.front().fitness() << ".\n";
        else
            cout << "Currunt Generation ->" << ++generation << " and the fitness is " << next_generation.front().fitness() << ".\n";
    }
    cout << next_generation.front();
}
int main()
{
    list<subject> subjects;
    list<teacher> teachers;
    list<classroom> classrooms;
    cout << "This program only works if there are enough teachers that the time table in mathematically possible.\n";
    cout << "enter the number of days the school is open.i.e 6 if mon to saturday.\n";
    cin >> number_of_days_school_is_open;
    cout << "enter the number of periods per day. Also if the first and second period must be taken by class teacher count them as one.\n";
    cin >> number_of_periods_per_day;
    cout << "Number of classes.\n";
    cin >> CLASSES;
    cout << "Is it compulsory that the first period be taken by the class teacher?(Enter y/n)\n";
    {
        char c;
        cin >> c;
        if (c == 'y' || c == 'Y')
            first_period_must_be_class_teacher = true;
        else if (c == 'n' || c == 'N')
            first_period_must_be_class_teacher = false;
        else
        {
            cout << "yaar tumko eak alphabet bhi theek se enter karna nahi ata ._. ? ab restart kro program -_-\n";
            return 0;
        }
    }

    //User enters the info of all subjects taught in school
    {
        string str;
        cout << "Enter the name of all the subjects that are taught in the instituion and enter -1 when the list has concluded.\n";
        cin >> ws;
        while (1)
        {
            getline(cin, str);
            if (str[0] == '-' && str[1] == '1')
                break;
            subjects.push_back(subject(str));
        }
    }
    //filling the list of teachers in the scope below
    {
        string str;
        for (list<subject>::iterator subjects_itrt = subjects.begin(); subjects_itrt != subjects.end(); subjects_itrt++)
        {
            cout << "Enter the name of all teachers who teach " << subjects_itrt->getter_subject_name() << " and enter -1 when the list has concluded.\n";
            while (true)
            {
                getline(cin, str);
                if (str.length() >= 2)
                    if (str[0] == '-' && str[1] == '1')
                        break;
                teachers.push_back(teacher(str, subjects_itrt));
                subjects_itrt->append_taught_by(--teachers.end());
            }
        }
    }

    //filling the list of all classrooms in the scope below
    {
        cout << "When entering the subjects+ their hours per week please make";
        cout << " sure that the sum of all hours is always strictly equal to ";
        cout << number_of_days_school_is_open * number_of_periods_per_day << "for the " << number_of_days_school_is_open << " days and the ";
        cout << number_of_periods_per_day << " periods.\n";
        cout << "In the counting include the first period as well i.e. if pooja devi takes homeroom and teacher 12 hours apart from homeroom enter 18 hours.\n";
        int class_number, random_number0, number_of_subjects, subject_id, subject_hours_per_week;
        char class_section;
        string str;
        while (true)
        {
            cout << "enter the next class and enter -1 if the list has extinguished.\n";
            cin >> class_number;
            if (class_number == -1)
                break;
            while (true)
            {
                cout << "enter the next section of class " << class_number << " and -1 if done.\n";
                cin >> str;
                if (str[0] == '-' && str[1] == '1')
                    break;
                class_section = str[0];
                cout << "who is the homeroom teacher of this class?Enter the number assosiated with them.\n";
                int random_variable1 = 0;
                for (list<teacher>::iterator rndm_it2 = teachers.begin(); rndm_it2 != teachers.end(); rndm_it2++)
                    cout << random_variable1++ << ")" << rndm_it2->getter_teacher_name() << " ";
                cout << endl;
                cin >> random_number0;
                list<teacher>::iterator rndm_itrt = teachers.begin();
                advance(rndm_itrt, random_number0);
                classrooms.push_back(classroom(class_number, class_section, rndm_itrt));
                rndm_itrt->is_homeroom_teacher(--classrooms.end()); //homeroom class/classteacher class has 0 index and is added right here
                if (first_period_must_be_class_teacher)
                    rndm_itrt->append_class_preference(--classrooms.end());
                cout << "Enter the number of subjects taught in" << class_number << " " << class_section << '\n';
                cin >> number_of_subjects;
                cout << "\nNow enter the id assosiated with that subject and the number of periods it is taught\n";
                random_variable1 = 0;
                for (list<subject>::iterator rndm_it2 = subjects.begin(); rndm_it2 != subjects.end(); rndm_it2++)
                    cout << random_variable1++ << ")" << rndm_it2->getter_subject_name() << " ";
                cout << endl;
                int checker_that_user_enters_the_correct_hours = 0;
                for (int i = 0; i < number_of_subjects; i++)
                {
                    cin >> subject_id >> subject_hours_per_week;
                    checker_that_user_enters_the_correct_hours += subject_hours_per_week;
                    list<subject>::iterator rndm_itrt1 = subjects.begin();
                    advance(rndm_itrt1, subject_id);
                    classrooms.back().append_subjects_and_hours_per_week(make_pair(rndm_itrt1, subject_hours_per_week));
                    rndm_itrt1->append_taught_in(--classrooms.end());
                }
                if (checker_that_user_enters_the_correct_hours != number_of_days_school_is_open * number_of_periods_per_day)
                {
                    cout << "You have entered the hours incorrectly since they don't add up to " << number_of_days_school_is_open * number_of_periods_per_day;
                    cout << '\n'
                         << "The program is being terminated.Please don't make this mistake again and enter carefully.\n";
                    return 0;
                }
            }
        }
    }
    time_table initial = time_table(subjects, teachers, classrooms);
    genetic_algorithm_for_time_table(initial);
    return 0;
}
