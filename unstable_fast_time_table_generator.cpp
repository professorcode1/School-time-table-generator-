#include <bits/stdc++.h>
#include <future>
using namespace std;
#define myfile cin
int number_of_periods_per_day, number_of_days_school_is_open, CLASSES, number_of_teacher;
int POPULATION_SIZE = 500, elite = 10, convergence_criteria = 50; // elite is in percentage
float mutation = 0.25, mutation_intensity = 0.01;                 //all these are in fraction
float acceptable_load_on_teacher = 0.76;
bool first_period_must_be_class_teacher;
std::mt19937 g2(std::chrono::system_clock::now().time_since_epoch().count());
int frequency_max_for_coulmn;

class classroom;
class teacher;
class subject
{
public:
    string subject_name;
    int subject_id;
    vector<teacher *> taught_by;
    vector<classroom *> taught_in;
    subject(string subject_name, int subject_id)
    {
        this->subject_name = subject_name;
        this->subject_id = subject_id;
    }
};
class classroom
{
public:
    int classroom_id, class_number;
    char class_section;
    teacher *homeroom_teacher;
    vector<pair<subject *, int>> subjects_and_hours_per_week;
    classroom(int number, char section, int id, teacher *homeroom)
    {
        classroom_id = id;
        class_number = number;
        class_section = section;
        homeroom_teacher = homeroom;
    }
};
class teacher
{
public:
    string teacher_name;
    int teacher_id;
    subject *teaches;
    classroom *homeroom_of;
    vector<classroom *> class_preference;
    bool **freedom;
    classroom ***teachers_time_table;
    teacher(string teacher_name, int teacher_id, subject *teaches)
    {
        this->teacher_name = teacher_name;
        this->teacher_id = teacher_id;
        this->teaches = teaches;
        homeroom_of = nullptr;

        freedom = new bool *[number_of_days_school_is_open];
        for (int i = 0; i < number_of_days_school_is_open; i++)
            freedom[i] = new bool[number_of_periods_per_day];

        for (int i = 0; i < number_of_days_school_is_open; i++)
            fill_n(freedom[i], number_of_periods_per_day, 1);

        teachers_time_table = new classroom **[number_of_days_school_is_open];
        for (int i = 0; i < number_of_days_school_is_open; i++)
            teachers_time_table[i] = new classroom *[number_of_periods_per_day];
        for (int i = 0; i < number_of_days_school_is_open; i++)
            for (int j = 0; j < number_of_periods_per_day; j++)
                teachers_time_table[i][j] = nullptr;
    }
    void not_free_for_first_period()
    {
        for (int j = 0; j < number_of_days_school_is_open; j++)
            freedom[j][0] = 0;
    }
    void print_teachers_time_table()
    {
        cout << teacher_name << endl;
        for (int j = 0; j < number_of_days_school_is_open; j++)
        {
            for (int k = 0; k < number_of_periods_per_day; k++)
            {
                if (teachers_time_table[j][k] == nullptr)
                    cout << "Free period    ";
                else
                {
                    int i = 3;
                    if (teachers_time_table[j][k]->class_number / 10 > 0)
                        i++;
                    cout << teachers_time_table[j][k]->class_number << " " << teachers_time_table[j][k]->class_section;
                    for (; i < 15; i++)
                        cout << " ";
                }
            }
            cout << '\n';
        }
        cout << '\n';
    }
};
class time_table
{
public:
    teacher ****table;
    list<classroom>::iterator classroom_iterator;
    time_table(int classes, list<classroom>::iterator classroom_iterator)
    {
        this->classroom_iterator = classroom_iterator;

        table = new teacher ***[classes];
        for (int i = 0; i < classes; i++)
            table[i] = new teacher **[number_of_days_school_is_open];

        for (int i = 0; i < classes; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                table[i][j] = new teacher *[number_of_periods_per_day];

        for (int i = 0; i < classes; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    table[i][j][k] = nullptr;
    }
    time_table(const time_table &object)
    {
        this->classroom_iterator = object.classroom_iterator;
        int classes = CLASSES;
        table = new teacher ***[classes];
        for (int i = 0; i < classes; i++)
            table[i] = new teacher **[number_of_days_school_is_open];

        for (int i = 0; i < classes; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                table[i][j] = new teacher *[number_of_periods_per_day];

        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    table[i][j][k] = object.table[i][j][k];
    }
    ~time_table()
    {
        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                delete[] table[i][j];
        for (int i = 0; i < CLASSES; i++)
            delete[] table[i];
        delete[] table;
    }
    time_table &operator=(const time_table &object)
    {
        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    table[i][j][k] = object.table[i][j][k];

        classroom_iterator = object.classroom_iterator;
        return *this;
    }
    bool operator==(const time_table &object)
    {
        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    if (table[i][j][k] != object.table[i][j][k])
                        return false;
        return true;
    }
    bool operator<(const time_table &object) const
    {
        /*Logic
			Every time table will have a numerical value associated with it. This value is given by
			SUMMATION i=0 to instances (number of teachers ^ (intances-i) * teacher id ). 
			So you can prove that this maps every distinct time table to a distinct number by first showing 
			that the number has base (number of teacher), all time table in this base are unique
			and are expressed in base 10 here.
 			*/
        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    if (table[i][j][k]->teacher_id > object.table[i][j][k]->teacher_id)
                        return false;
        return true;
    }
    void complete_the_teachers_time_table()
    {
        list<classroom>::iterator classroom_iterator1 = classroom_iterator;
        for (int i = 0; i < CLASSES; i++)
        {
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                {
                    table[i][j][k]->teachers_time_table[j][k] = &*classroom_iterator1;
                }
            classroom_iterator1++;
        }
    }
};
ostream &operator<<(ostream &out, const teacher &object)
{
    out << "Teacher name is " << object.teacher_name << ".\nTheir ID is " << object.teacher_id << ".\n";
    out << "The subject they teach is " << object.teaches->subject_name << "\n";
    if (object.homeroom_of != nullptr)
        out << "They are the homeroom of " << object.homeroom_of->class_number << " " << object.homeroom_of->class_section << '\n';
    out << "The classes they teach are :";
    for (int i = 0; i < object.class_preference.size(); i++)
        out << object.class_preference[i]->class_number << object.class_preference[i]->class_section << " ";
    out << "\n\n";
    return out;
}
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
    if (object.homeroom_teacher != 0)
        out << "The homeroom teacher is " << object.homeroom_teacher->teacher_name << ".\n";
    out << "The subjects taught per hour are \n";
    for (int i = 0; i < object.subjects_and_hours_per_week.size(); i++)
        out << "Name:" << object.subjects_and_hours_per_week[i].first->subject_name << " Hours:" << object.subjects_and_hours_per_week[i].second << "\n";
    out << "\n";
    return out;
}
ostream &operator<<(ostream &out, const time_table &object)
{
    list<classroom>::iterator rndm_it0 = object.classroom_iterator;
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
                if (object.table[i][j][k] == nullptr)
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
void set_frequnecy_max_for_coulmns(const time_table &obj)
{
    frequency_max_for_coulmn = 0; //for every coulmns aka period number, the number of distinct teacher will be evaluated
    int arr[number_of_days_school_is_open];
    for (int i = 0; i < CLASSES; i++)
    {
        for (int k = 0; k < number_of_periods_per_day; k++)
        {
            for (int j = 0; j < number_of_days_school_is_open; j++)
                arr[j] = obj.table[i][j][k]->teacher_id;
            sort(arr, arr + number_of_days_school_is_open);
            int freq = 1;
            for (int j = 1; j < number_of_days_school_is_open; j++)
                if (arr[j] != arr[j - 1])
                    freq++;
            if (freq == 1)
                frequency_max_for_coulmn++;
        }
    }
}
int clashes(const time_table &obj)
{
    int clashes = 0;
    bool yes;
    for (int j = 0; j < number_of_days_school_is_open; j++)
    {
        for (int k = 0; k < number_of_periods_per_day; k++)
        {
            for (int i = 0; i < CLASSES; i++)
            {
                yes = true;
                for (int x = i - 1; x >= 0; x--)
                    if (obj.table[i][j][k] == obj.table[x][j][k])
                        yes = false;
                bool yes1 = false;
                if (yes)
                    for (int x = i + 1; x < CLASSES; x++)
                        if (obj.table[i][j][k] == obj.table[x][j][k])
                        {
                            clashes++;
                            yes1 = true;
                        }
                if (yes1)
                    clashes++;
            }
        }
    }
    return clashes;
}
double fitness(const time_table &obj)
{
    int frequency_for_coulmns, frequency_for_rows;
    int clashes = 0;
    double ans = 0;

    bool yes;
    for (int j = 0; j < number_of_days_school_is_open; j++)
    {
        for (int k = 0; k < number_of_periods_per_day; k++)
        {
            for (int i = 0; i < CLASSES; i++)
            {
                yes = true;
                for (int x = i - 1; x >= 0; x--)
                    if (obj.table[i][j][k] == obj.table[x][j][k])
                        yes = false;
                bool yes1 = false;
                if (yes)
                    for (int x = i + 1; x < CLASSES; x++)
                        if (obj.table[i][j][k] == obj.table[x][j][k])
                        {
                            clashes++;
                            yes1 = true;
                        }
                if (yes1)
                    clashes++;
            }
        }
    }
    //return static_cast<double>(-1*clashes);

    //students study different classes and not same class consequtively
    frequency_for_coulmns = 0;
    int arr[number_of_days_school_is_open];
    for (int i = 0; i < CLASSES; i++)
    {
        for (int k = 0; k < number_of_periods_per_day; k++)
        {
            for (int j = 0; j < number_of_days_school_is_open; j++)
                arr[j] = obj.table[i][j][k]->teacher_id;
            sort(arr, arr + number_of_days_school_is_open);
            int freq = 1;
            for (int j = 1; j < number_of_days_school_is_open; j++)
                if (arr[j] != arr[j - 1])
                    freq++;
            if (freq == 1)
                frequency_for_coulmns++;
        }
    }

    //teachers teach the same class same periods on different days.
    frequency_for_rows = 0;
    for (int i = 0; i < CLASSES; i++)
    {
        for (int j = 0; j < number_of_days_school_is_open; j++)
        {
            int freq = 1;
            for (int k = 1; k < number_of_periods_per_day; k++)
                if (obj.table[i][j][k] != obj.table[i][j][k - 1])
                {
                    if (freq == 1)
                        frequency_for_rows++;
                    freq = 1;
                }
                else
                    freq++;
            if (freq == 1)
                frequency_for_rows++;
        }
    }
    //in the NUMERATOR in ans frequency max for coulmns is used. This effectively turns that heuristic off. Turning it on makes the time table unperiodic and random
    if (1)
        ans = static_cast<double>(100 * frequency_for_rows * frequency_max_for_coulmn) / static_cast<double>(frequency_max_for_coulmn * CLASSES * number_of_periods_per_day * number_of_days_school_is_open);
    else
        ans = 0;
    ans += static_cast<double>(-100 * clashes);
    return ans;
}
void perform_permutation(vector<int *> &vec, int arr[], int l, int r)
{
    if (l == r - 1)
    {
        int *x = new int[r];
        copy(arr, arr + r, x);
        vec.push_back(x);
    }
    else
    {
        for (int i = l; i < number_of_periods_per_day; i++)
        {
            swap(arr[i], arr[l]);
            perform_permutation(vec, arr, l + 1, r);
            swap(arr[i], arr[l]);
        }
    }
}
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
void mutate(time_table &object, int l)
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
                            first_conflicted = first_conflicted || object.table[x][j][k] == object.table[i][j][k];
                    for (int x = 0; x < CLASSES; x++)
                        if (x == i)
                            continue;
                        else
                            second_conflicted = second_conflicted || object.table[x][second_index % number_of_days_school_is_open][second_index / number_of_days_school_is_open] == object.table[i][second_index % number_of_days_school_is_open][second_index / number_of_days_school_is_open];
                    float mutation_number = mutation_intensity;
                    if (first_conflicted || second_conflicted)
                        mutation_number *= 100;
                    if (first_period_must_be_class_teacher && second_index % number_of_days_school_is_open == 0)
                        continue;
                    if (sudorandom_number_generator(0, 9999) <= ceil(10000 * mutation_number))
                    {
                        swap(object.table[i][j][k], object.table[i][second_index % number_of_days_school_is_open][second_index / number_of_days_school_is_open]);
                        return;
                    }
                }
}
int tournament_selection()
{
    long long l = 1, r = (static_cast<long long>(POPULATION_SIZE) * static_cast<long long>(POPULATION_SIZE + 1)) / 2;
    long long length = r - l + 1;
    long long z = (g2() % length) + l;
    long double alpha = 8 * (r - z) + 1;
    alpha = sqrtl(alpha) + 1;
    alpha /= 2;
    long long x = POPULATION_SIZE - floor(alpha);
    if (x < 0 || x >= POPULATION_SIZE)
    {
        cout << "tournament_selection is not working.\n";
        cin.get();
    }
    return int(x);
}
void tabu_search(time_table *object, double objects_fitness)
{
    /*Here how the tabu tenure is determined per iteration
	If the algorithm want to tend towards deadlock then what is the minimum tabu tenure it needs? That value is the upper bound
	To calculate if a time table for a class has x1 classes of some subject and x2 of another and so on till x6 if there are 6 subjects
	Now for choosing a period to lock we have x2+x2...x6 options for x1. Basically class vise we have 48C2 - (x1)C2 -(x2)C2...-(x6)C2 possibilites.
	This value is maximised when standard deviation b/s xi's in minimum. i.e each x1 = floor(periods/number of subjects)
	Now multiply that with number of classes and  we have an upper bound. 
	Reduce that upper bound to 0.9 of its intial value for realistic implimentation
	0.01 of the realistc value is the lower bound.
	Now ossilate b/w them by multiplying with 0.9991 to go lower and 1.0001 to go upper.
	Switch the tabu tenure per iteration by the above formula while keeping it in bounds. 
	*/
    int number_of_periods = number_of_periods_per_day * number_of_days_school_is_open;
    vector<vector<vector<int>>> tabu_tenure_per_element(CLASSES, vector<vector<int>>(number_of_days_school_is_open, vector<int>(number_of_periods_per_day)));
    for (int i = 0; i < CLASSES; i++)
        for (int j = 0; j < number_of_days_school_is_open; j++)
            for (int k = 0; k < number_of_periods_per_day; k++)
                tabu_tenure_per_element[i][j][k] = 0;
    float upper_limit, lower_limit, current_tabu_tenure;
    float decrease_factor = 0.96, increase_factor = 1.04;
    bool decreasing = true;
    //lets calculate upper limit and derive lower limit
    {
        /*upper_limit = CLASSES * (number_of_periods * (number_of_periods - 1) / 2);
		set<int> number_of_subjects;
		for (int i = 0; i < CLASSES; i++)
		{
			number_of_subjects.clear();
			for (int j = 0; j < number_of_days_school_is_open; j++)
				for (int k = 0; k < number_of_periods_per_day; k++)
					number_of_subjects.insert(object->table[i][j][k]->teaches->subject_id);
			int num_of_sub = number_of_subjects.size();
			upper_limit -= (number_of_periods / num_of_sub) * ((number_of_periods / num_of_sub) - 1) * (num_of_sub);
		}*/
        upper_limit = CLASSES * number_of_periods / 2;
        lower_limit = 10;
    }
    //Completed the lower and upper bound on tabu tenure
    current_tabu_tenure = upper_limit;
    time_table sBest = *object, sBestcandidate = *object;
    int initial_fitness = -1 * clashes(*object);
    int best_so_far = initial_fitness, best_candidate = initial_fitness;
    int swap_element1[3] = {0}, swap_element2[3] = {0};

    while (1)
    {
        for (int i = 0; i < CLASSES; i++)
            for (int j = 0; j < number_of_days_school_is_open; j++)
                for (int k = 0; k < number_of_periods_per_day; k++)
                    if (tabu_tenure_per_element[i][j][k] > 0)
                        tabu_tenure_per_element[i][j][k]--;
        time_table neighbour = sBestcandidate;
        int first_neighbor_i = -1, first_neighbor_x1 = -1, first_neighbor_x2 = -1;
        for (int i = 0; i < CLASSES; i++)
            for (int x1 = first_period_must_be_class_teacher ? number_of_days_school_is_open : 0; x1 < number_of_periods - 1; x1++)
                for (int x2 = x1 + 1; x2 < number_of_periods; x2++)
                    if (neighbour.table[i][x1 % number_of_days_school_is_open][x1 / number_of_days_school_is_open] != neighbour.table[i][x2 % number_of_days_school_is_open][x2 / number_of_days_school_is_open] && tabu_tenure_per_element[i][x2 % number_of_days_school_is_open][x2 / number_of_days_school_is_open] == 0 && tabu_tenure_per_element[i][x1 % number_of_days_school_is_open][x1 / number_of_days_school_is_open] == 0)
                    {
                        first_neighbor_i = i;
                        first_neighbor_x1 = x1;
                        first_neighbor_x2 = x2;
                    }
        if (first_neighbor_i == -1) //deadlock has occured
            continue;
        swap(neighbour.table[first_neighbor_i][first_neighbor_x1 % number_of_days_school_is_open][first_neighbor_x1 / number_of_days_school_is_open], neighbour.table[first_neighbor_i][first_neighbor_x2 % number_of_days_school_is_open][first_neighbor_x2 / number_of_days_school_is_open]);
        best_candidate = -1 * clashes(neighbour);
        swap(neighbour.table[first_neighbor_i][first_neighbor_x1 % number_of_days_school_is_open][first_neighbor_x1 / number_of_days_school_is_open], neighbour.table[first_neighbor_i][first_neighbor_x2 % number_of_days_school_is_open][first_neighbor_x2 / number_of_days_school_is_open]);
        //in the below scope neighborhood will be transversed.
        {
            for (int i = 0; i < CLASSES; i++)
                for (int x1 = first_period_must_be_class_teacher ? number_of_days_school_is_open : 0; x1 < number_of_periods - 1; x1++)
                    for (int x2 = x1 + 1; x2 < number_of_periods; x2++)
                        if (neighbour.table[i][x1 % number_of_days_school_is_open][x1 / number_of_days_school_is_open] != neighbour.table[i][x2 % number_of_days_school_is_open][x2 / number_of_days_school_is_open])
                        {
                            pair<int, int> index1 = make_pair(x1 % number_of_days_school_is_open, x1 / number_of_days_school_is_open), index2 = make_pair(x2 % number_of_days_school_is_open, x2 / number_of_days_school_is_open);
                            swap(neighbour.table[i][index1.first][index1.second], neighbour.table[i][index2.first][index2.second]);
                            int this_neighbors_fitnesss = -1 * clashes(neighbour);
                            if (this_neighbors_fitnesss > best_so_far)
                            {
                                sBestcandidate = neighbour;

                                best_candidate = this_neighbors_fitnesss;

                                swap_element1[0] = i;
                                swap_element1[1] = index1.first;
                                swap_element1[2] = index1.second;

                                swap_element2[0] = i;
                                swap_element2[1] = index2.first;
                                swap_element2[2] = index2.second;
                            }
                            if (this_neighbors_fitnesss >= best_candidate && tabu_tenure_per_element[i][index1.first][index1.second] == 0 && tabu_tenure_per_element[i][index2.first][index2.second] == 0)
                            {
                                sBestcandidate = neighbour;

                                best_candidate = this_neighbors_fitnesss;

                                swap_element1[0] = i;
                                swap_element1[1] = index1.first;
                                swap_element1[2] = index1.second;

                                swap_element2[0] = i;
                                swap_element2[1] = index2.first;
                                swap_element2[2] = index2.second;
                            }
                            swap(neighbour.table[i][index1.first][index1.second], neighbour.table[i][index2.first][index2.second]);
                        }
        }

        if (best_so_far > initial_fitness && best_candidate <= best_so_far)
            break;

        if (best_candidate > best_so_far)
        {
            sBest = sBestcandidate;
            best_so_far = best_candidate;
        }
        cout << "          " << best_candidate << endl;

        tabu_tenure_per_element[swap_element1[0]][swap_element1[1]][swap_element1[2]] = current_tabu_tenure;
        tabu_tenure_per_element[swap_element2[0]][swap_element2[1]][swap_element2[2]] = current_tabu_tenure;

        if (decreasing)
            current_tabu_tenure = floor(decrease_factor * current_tabu_tenure);
        else
            current_tabu_tenure = ceil(increase_factor * current_tabu_tenure);
        if (current_tabu_tenure >= upper_limit || current_tabu_tenure <= lower_limit)
            decreasing = !decreasing;
        cout << current_tabu_tenure << endl;
    }
    *object = sBest;
}
void genetic_algorithm_for_time_table(list<subject> subjects, list<teacher> teachers, list<classroom> classrooms)
{
    time_table complete_time_table(CLASSES, classrooms.begin());
    if (first_period_must_be_class_teacher)
        for (list<classroom>::iterator rndm_it0 = classrooms.begin(); rndm_it0 != classrooms.end(); rndm_it0++)
        {
            rndm_it0->homeroom_teacher->not_free_for_first_period();
            for (int j = 0; j < number_of_days_school_is_open; j++)
                complete_time_table.table[distance(classrooms.begin(), rndm_it0)][j][0] = rndm_it0->homeroom_teacher;
        }
    vector<pair<time_table *, double>> population_of_next_generation;
    //FIRST STEP : creating the intial population. The below scope will do that
    {
        vector<vector<pair<teacher *, int>>> teacher_and_hours(CLASSES);
        //seeing what teacher teaches how many hours in a class will be determined below
        {
            for (list<teacher>::iterator rndm_it0 = teachers.begin(); rndm_it0 != teachers.end(); rndm_it0++)
            {
                for (const auto x : rndm_it0->class_preference)
                {
                    int hours;
                    for (const auto y : x->subjects_and_hours_per_week)
                        if (y.first == rndm_it0->teaches)
                            hours = y.second;
                    teacher_and_hours.at(x->classroom_id).push_back(make_pair(&*rndm_it0, hours));
                }
            }
        }
        //using that information the first  offstring, the first of his kind will be produced beloow
        int anti_symmetric_index[CLASSES];
        {
            for (int i = 0; i < CLASSES; i++)
            {
                int j = first_period_must_be_class_teacher ? 1 : 0;
                for (const auto x : teacher_and_hours[i])
                {
                    int y = x.second / number_of_days_school_is_open;
                    while (y > 0)
                    {
                        for (int k = 0; k < number_of_days_school_is_open; k++)
                            complete_time_table.table[i][k][j] = x.first;
                        y--;
                        j++;
                    }
                }
                anti_symmetric_index[i] = j;
                int l = j * number_of_days_school_is_open;
                for (const auto x : teacher_and_hours[i])
                {
                    int y = x.second % number_of_days_school_is_open;
                    while (y > 0)
                    {
                        complete_time_table.table[i][l % number_of_days_school_is_open][l / number_of_days_school_is_open] = x.first;
                        l++;
                        y--;
                    }
                }
            }
        }

        set_frequnecy_max_for_coulmns(complete_time_table);
        //tabu_search(&complete_time_table, 1);
        //and by permuting this offspring the rest of the population will be produces
        {
            /*so heres the plan. Produce all permutations of Number_of_periods_per_daythen see in this new permutation which rows have value
			greater than = stores in assymtrix index array. use those index to jumble them randomly. and voila, done */
            int arr[number_of_periods_per_day];
            for (int i = 0; i < number_of_periods_per_day; i++)
                arr[i] = i;
            vector<int *> permutation;
            if (first_period_must_be_class_teacher)
                perform_permutation(permutation, arr, 1, number_of_periods_per_day);
            else
                perform_permutation(permutation, arr, 0, number_of_periods_per_day);

            for (int i = 0; i < permutation.size(); i++)
                swap(permutation[i], permutation.at(sudorandom_number_generator(i, permutation.size() - 1)));
            for (int i = 0; i < POPULATION_SIZE; i++)
                population_of_next_generation.push_back(make_pair(new time_table(CLASSES, classrooms.begin()), 0));
            int percent = 0, rndm_nmbr = 0;
            for (int i = 0; i < CLASSES; i++)
            {
                for (int population_passer = 0; population_passer < POPULATION_SIZE; population_passer++)
                {
                    int percentage_complete = floor(static_cast<float>((i * POPULATION_SIZE) + population_passer) * 100 / (POPULATION_SIZE * CLASSES));
                    if (percentage_complete > percent)
                    {
                        printf("%dpercent done.\n", percentage_complete);
                        percent = percentage_complete;
                    }
                    rndm_nmbr = (rndm_nmbr + 1) % permutation.size();
                    vector<pair<pair<int, int>, pair<int, int>>> anti_symmetric_swapper;
                    for (int j = 0; j < number_of_days_school_is_open; j++)
                    {
                        for (int k = 0; k < number_of_periods_per_day; k++)
                        {
                            population_of_next_generation[population_passer].first->table[i][j][k] = complete_time_table.table[i][j][permutation.at(rndm_nmbr)[k]];
                            if (permutation.at(rndm_nmbr)[k] >= anti_symmetric_index[i])
                                anti_symmetric_swapper.push_back(make_pair(make_pair(j, permutation.at(rndm_nmbr)[k]), make_pair(j, permutation.at(rndm_nmbr)[k])));
                        }
                    }
                    for (int rndm_var = 0; rndm_var < anti_symmetric_swapper.size(); rndm_var++)
                        swap(anti_symmetric_swapper[rndm_var].first, anti_symmetric_swapper[sudorandom_number_generator(rndm_var, anti_symmetric_swapper.size() - 1)].first);
                    for (const auto x : anti_symmetric_swapper)
                        swap(population_of_next_generation[population_passer].first->table[i][x.first.first][x.first.second], population_of_next_generation[population_passer].first->table[i][x.second.first][x.second.second]);
                }
            }
            for (int i = 0; i < permutation.size(); i++)
                delete[] permutation[i];
        }
        for (int i = 0; i < POPULATION_SIZE; i++)
            population_of_next_generation[i].second = fitness(*population_of_next_generation[i].first);
        sort(population_of_next_generation.begin(), population_of_next_generation.end(), [](const pair<time_table *, int> &l, const pair<time_table *, int> &r) { return l.second > r.second; });
    }
    vector<pair<time_table *, double>> population_of_previous_generation;

    int checking_if_time_table_is_possible[teachers.size()] = {0};
    for (int i = 0; i < CLASSES; i++)
        for (int j = 0; j < number_of_days_school_is_open; j++)
            for (int k = 0; k < number_of_periods_per_day; k++)
                checking_if_time_table_is_possible[complete_time_table.table[i][j][k]->teacher_id]++;
    for (int i = 0; i < teachers.size(); i++)
        if (checking_if_time_table_is_possible[i] <= ceil(static_cast<float>(number_of_periods_per_day * number_of_days_school_is_open) * acceptable_load_on_teacher))
            0;
        else
        {
            list<teacher>::iterator it1 = teachers.begin();
            cout << "Wait time table is not possible? ...*Astranuat Pointing gun *Never has been " << endl;
            advance(it1, i);
            if (checking_if_time_table_is_possible[i] > number_of_days_school_is_open * number_of_periods_per_day)
            {
                cout << "The teacher " << it1->teacher_name << " teaches more than is litreally possible.\n aka more than ";
                cout << number_of_periods_per_day * number_of_days_school_is_open << "She is teaching";
                cout << checking_if_time_table_is_possible[i] << '\n'
                     << " Reduce durden on her by evenly distriburing ";
                cout << "subjects and it they are evenly distributed hire more teachers . To use the software in the list enter a hypothetical teacher.\n";
                return;
            }
            else
            {
                cout << "The teacher " << it1->teacher_name << " has an unrealisitic burden . You can procced with the program but its unlikely you will get an answer.\n";
                cout << "Press enter to continue.\n";
                cin.get();
            }
        }
    //Now evolve it untill we converge at a global optimum
    {
        int generation = 0;
        double convergence_value = population_of_next_generation[0].second, convergence_gen = generation;
        cout << "Currunt Generation ->" << ++generation << " and the minimum number of clashes is " << -1 * population_of_next_generation[0].second << ".\n";
        bool convergence_has_been_reached = false;
        while (population_of_next_generation[0].second < 0 || abs(convergence_gen - generation) < convergence_criteria)
        {

            population_of_previous_generation.clear();
            for (int itrt = 0; itrt < POPULATION_SIZE; itrt++)
                population_of_previous_generation.push_back(make_pair(new time_table(*population_of_next_generation[itrt].first), population_of_next_generation[itrt].second));
            //NEXT GEN HAS BEEN TRANSFERED TO PREVIOUS GEN

            for (int itrt = 0; itrt < POPULATION_SIZE; itrt++)
                delete population_of_next_generation[itrt].first;
            population_of_next_generation.clear();
            int elites_passed_down = ceil((static_cast<float>(POPULATION_SIZE) * elite) / 100);
            for (int itrt = 0; itrt < elites_passed_down; itrt++)
                population_of_next_generation.push_back(make_pair(new time_table(*population_of_previous_generation[itrt].first), -1000));
            //THE ELITES FROM PREVIOUS GEN HAVE BEEN ADDED TO THIS GEN

            for (int itrt = elites_passed_down; itrt < POPULATION_SIZE; itrt++)
            {
                int index1 = tournament_selection(), index2 = tournament_selection();
                for (int i = 0; i < CLASSES; i++)
                {
                    int index = (sudorandom_number_generator(0, 1)) ? index1 : index2;
                    for (int j = 0; j < number_of_days_school_is_open; j++)
                        for (int k = 0; k < number_of_periods_per_day; k++)
                            complete_time_table.table[i][j][k] = population_of_previous_generation.at(index).first->table[i][j][k];
                }
                population_of_next_generation.push_back(make_pair(new time_table(complete_time_table), -1000));
            }
            for (int itrt = 0; itrt < POPULATION_SIZE; itrt++)
                population_of_next_generation[itrt].second = fitness(*population_of_next_generation[itrt].first);
            //CROSSOVER TOURNAMENT PERFORMED AND NEXT GEN CREATED

            int number_of_elemnets_mutated = ceil(POPULATION_SIZE * mutation);
            for (int itrt = 0; itrt < number_of_elemnets_mutated; itrt++)
            {
                int level = sudorandom_number_generator(0, CLASSES - 1);
                int index = sudorandom_number_generator(0, POPULATION_SIZE - 1);
                mutate(*population_of_next_generation[index].first, level);
                population_of_next_generation[index].second = fitness(*population_of_next_generation[index].first);
            }

            sort(population_of_next_generation.begin(), population_of_next_generation.end(), [](const pair<time_table *, int> &l, const pair<time_table *, int> &r) { return l.second > r.second; });

            // if (generation - convergence_gen >= convergence_criteria)
            // {
            // 	cout << "convergence has occured.\n";
            // 	for (int i = 0; i < ceil(POPULATION_SIZE * elite / 100); i++)
            // 	{
            // 		cout << i << endl;
            // 		tabu_search(population_of_next_generation[i].first, population_of_next_generation[i].second);
            // 		population_of_next_generation[i].second = fitness(*population_of_next_generation[i].first);
            // 		if (population_of_next_generation[i].second > 0)
            // 			break;
            // 	}
            // 	sort(population_of_next_generation.begin(), population_of_next_generation.end(), [](const pair<time_table *, int> &l, const pair<time_table *, int> &r) { return l.second > r.second; });
            // 	if (population_of_next_generation[0].second > 0)
            // 		break;
            // }

            if (population_of_next_generation[0].second < 0)
                cout << "Currunt Generation ->" << ++generation << " and the sickness is " << -1 * population_of_next_generation[0].second << ".\n";
            else
                cout << "Currunt Generation ->" << ++generation << " and the fitness is " << population_of_next_generation[0].second << ".\n";

            for (int itrt = 0; itrt < POPULATION_SIZE; itrt++)
                delete population_of_previous_generation[itrt].first;

            if (floor(population_of_next_generation[0].second / 100) != convergence_value)
            {
                convergence_value = floor(population_of_next_generation[0].second / 100);
                convergence_gen = generation;
            }
        }
    }
    complete_time_table = *population_of_next_generation.front().first;
    for (int i = 1; i < POPULATION_SIZE; i++)
        delete population_of_next_generation[i].first;

    complete_time_table.complete_the_teachers_time_table();
    cout << complete_time_table;
    for (auto teacher1 : teachers)
        teacher1.print_teachers_time_table();
}
int main()
{
    srand(time(0));
    /*
	ifstream myfile ("data.txt");
	if(myfile.is_open())
	cout<<"Yeah!open!"<<endl;
	*/
    cout << "enter the number of days the school is open.i.e 6 if mon to saturday.\n";
    cin >> number_of_days_school_is_open;
    cout << "enter the number of periods per day. Also if the first and second period must be taken by class teacher count them as one.\n";
    cin >> number_of_periods_per_day;
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
    list<subject> subjects;
    //creating a doubly linked list (or somply list) of all the subjects that are taught in the insitution in below scope
    {
        int id = 0;
        string str;
        cout << "Enter the name of all the subjects that are taught in the instituion and enter -1 when the list has concluded.\n";
        cin >> ws;
        while (1)
        {
            getline(myfile, str);
            if (str[0] == '-' && str[1] == '1')
                break;
            subjects.push_back(subject(str, id));
            cout << str << endl;
            id++;
        }
    }

    system("CLS");
    list<teacher> teachers;
    //filling the list of teachers in the scope below
    {
        int id = 0;
        string str;
        for (int i = 0; i < subjects.size(); i++)
        {
            list<subject>::iterator rndm_it0 = subjects.begin();
            advance(rndm_it0, i);
            cout << "enter the name of all teacher who teach " << rndm_it0->subject_name << " and enter -1 when the list has concluded.\n";
            while (1)
            {
                getline(myfile, str);
                if (str[0] == '-' && str[1] == '1')
                    break;
                teachers.push_back(teacher(str, id, &*rndm_it0));
                rndm_it0->taught_by.push_back(&teachers.back());
                id++;
            }
        }
    }

    system("CLS");
    list<classroom> classrooms;
    //filling the classes in the scope below
    {
        cout << "When entering the subjects+ their hours per week please make";
        cout << " sure that the sum of all hours is always strictly equal to ";
        cout << number_of_days_school_is_open * number_of_periods_per_day << "for the " << number_of_days_school_is_open << " days and the ";
        cout << number_of_periods_per_day << " periods.\n";
        cout << "In the counting include the first period as well i.e. if pooja devi takes homeroom and teacher 12 hours apart from homeroom enter 18 hours.\n";
        int id(0), class_number, random_number0, number_of_subjects, subject_id, subject_hours_per_week;
        char class_section;
        string str;
        while (1)
        {
            cout << "enter the next class and enter -1 if the list has extinguished.\n";
            myfile >> class_number;
            if (class_number == -1)
                break;
            while (1)
            {
                cout << "enter the next section of class " << class_number << " and -1 if done.\n";
                myfile >> str;
                if (str[0] == '-' && str[1] == '1')
                    break;
                class_section = str[0];
                cout << "who is the homeroom teacher of this class?Enter the number assosiated with them.\n";
                int random_variable1 = 0;
                for (list<teacher>::iterator rndm_it2 = teachers.begin(); rndm_it2 != teachers.end(); rndm_it2++)
                    cout << ++random_variable1 << ")" << rndm_it2->teacher_name << " ";
                cout << endl;
                myfile >> random_number0;
                list<teacher>::iterator rndm_it0 = teachers.begin();
                advance(rndm_it0, random_number0 - 1);
                classrooms.push_back(classroom(class_number, class_section, id++, &*rndm_it0));
                rndm_it0->homeroom_of = &classrooms.back();
                rndm_it0->class_preference.push_back(&classrooms.back());
                random_variable1 = 0;
                for (list<subject>::iterator rndm_it2 = subjects.begin(); rndm_it2 != subjects.end(); rndm_it2++)
                    cout << ++random_variable1 << ")" << rndm_it2->subject_name << " ";
                cout << endl;
                cout << "enter the number of subjects taguht in" << class_number << " " << class_section << ". Then enter their id's and how many hours they're taught\n";
                myfile >> number_of_subjects;
                int checker_that_user_enters_the_correct_hours = 0;
                for (int i = 0; i < number_of_subjects; i++)
                {
                    myfile >> subject_id >> subject_hours_per_week;
                    checker_that_user_enters_the_correct_hours += subject_hours_per_week;
                    list<subject>::iterator rndm_it1 = subjects.begin();
                    advance(rndm_it1, subject_id - 1);
                    if (classrooms.back().homeroom_teacher->teaches == &*rndm_it1 && first_period_must_be_class_teacher)
                        classrooms.back().subjects_and_hours_per_week.push_back(make_pair(&*rndm_it1, subject_hours_per_week - number_of_days_school_is_open));
                    else
                        classrooms.back().subjects_and_hours_per_week.push_back(make_pair(&*rndm_it1, subject_hours_per_week));
                    rndm_it1->taught_in.push_back(&classrooms.back());
                }
                if (checker_that_user_enters_the_correct_hours != number_of_days_school_is_open * number_of_periods_per_day)
                {
                    cout << "You have entered the hours incorrectly since they don't add up to " << number_of_days_school_is_open * number_of_periods_per_day;
                    cout << '\n'
                         << "The program is being terminated.Please don't make this mistake again and enter carefully.\n";
                    return 0;
                }
                system("CLS");
            }
        }
    }
    system("CLS");
    cout << "Now all the information you have entered so far will be displayed.\n";
    cout << "Press enter to continue.\n";
    cin.get();
    for (list<subject>::iterator i = subjects.begin(); i != subjects.end(); i++)
        cout << *i;
    cout << "Press enter to continue.\n";
    cin.get();
    for (list<teacher>::iterator i = teachers.begin(); i != teachers.end(); i++)
        cout << *i;
    cout << "Press enter to continue.\n";
    cin.get();
    for (list<classroom>::iterator i = classrooms.begin(); i != classrooms.end(); i++)
        cout << *i;
    cout << "Press enter to continue.\n";
    cin.get();
    system("CLS");
    cout << "Here's how the classes are assigned.\n";
    //in the below scope teacher preference will be decided
    {
        vector<pair<classroom *, teacher *>> vec;
        bool complete;
        int class_index, min_class, satisfied, quierys, parameter1, parameter2, parameter3;
        teacher *free_teacher;
        for (list<subject>::iterator rndm_it0 = subjects.begin(); rndm_it0 != subjects.end(); rndm_it0++)
        {
            vec.clear();
            for (vector<classroom *>::iterator rndm_it1 = rndm_it0->taught_in.begin(); rndm_it1 != rndm_it0->taught_in.end(); rndm_it1++)
            {
                if ((*rndm_it1)->homeroom_teacher->teaches == (&*rndm_it0))
                    vec.push_back(make_pair((*rndm_it1), (*rndm_it1)->homeroom_teacher));
                else
                    vec.push_back(make_pair(*rndm_it1, nullptr));
            }
            //last classroom first teacher. Thats how they have to be selected
            complete = true;
            for (int i = 0; i < vec.size(); i++)
                if (vec[i].second == nullptr)
                {
                    complete = false;
                    class_index = i;
                }

            while (not complete)
            {
                free_teacher = &*(rndm_it0->taught_by.front());
                min_class = rndm_it0->taught_by.front()->class_preference.size();
                for (int i = 0; i < rndm_it0->taught_by.size(); i++)
                    if (rndm_it0->taught_by[i]->class_preference.size() < min_class)
                    {
                        min_class = rndm_it0->taught_by[i]->class_preference.size();
                        free_teacher = rndm_it0->taught_by[i];
                    }

                vec[class_index].second = free_teacher;
                free_teacher->class_preference.push_back(vec[class_index].first);

                complete = true;
                for (int i = 0; i < vec.size(); i++)
                    if (vec[i].second == nullptr)
                    {
                        complete = false;
                        class_index = i;
                    }
            }
            for (int i = 0; i < rndm_it0->taught_by.size(); i++)
            {
                cout << rndm_it0->taught_by[i]->teacher_id << ")" << rndm_it0->taught_by[i]->teacher_name << "->" << '\t';
                for (int j = 0; j < rndm_it0->taught_by[i]->class_preference.size(); j++)
                    cout << rndm_it0->taught_by[i]->class_preference[j]->classroom_id << ")" << rndm_it0->taught_by[i]->class_preference[j]->class_number << rndm_it0->taught_by[i]->class_preference[j]->class_section << "  ";
                cout << '\n';
            }
            cout << "Enter 1 if you are okay with that distribution and enter -1 if you are not okay.\n";
            myfile >> satisfied;
            if (satisfied == -1)
            {
                cout << "Since you chose to alter the solution here's how you alter it.First enter the number of quiries" << endl;
                cout << "For each quiery Choose a teacher id,then a class id,then a teacher id. \n";
                cout << "The teacher you first chose will have the chosen class transfered to the teacher who's id you entred second\n";
                myfile >> quierys;
                while (quierys--)
                {
                    myfile >> parameter1 >> parameter2 >> parameter3;
                    //parameter1--;
                    //parameter2--;
                    //parameter3--;
                    list<teacher>::iterator rndm_it1 = teachers.begin();
                    advance(rndm_it1, parameter1);
                    list<classroom>::iterator rndm_it2 = classrooms.begin();
                    advance(rndm_it2, parameter2);
                    list<teacher>::iterator rndm_it3 = teachers.begin();
                    advance(rndm_it3, parameter3);
                    classroom *transfer_class = &*rndm_it2;
                    vector<classroom *>::iterator rndm_it4 = find(rndm_it1->class_preference.begin(), rndm_it1->class_preference.end(), transfer_class);
                    if (rndm_it4 == rndm_it1->class_preference.end())
                    {
                        cout << "quiery is invalid as the teacher already doesn't have this class.\n reenter the quiery carefully'\n";
                        quierys++;
                        continue;
                    }
                    if (rndm_it1->homeroom_of == &*rndm_it2)
                    {
                        cout << "quiery is invalid. The first teacher you picked is the homeroom of the class you picked.\n";
                        cout << "reenter you quiery carefully.\n";
                        quierys++;
                        continue;
                    }
                    rndm_it1->class_preference.erase(rndm_it4);
                    rndm_it3->class_preference.push_back(&*rndm_it2);
                }
                cout << "Now this is the final distribution.\n";
                for (int i = 0; i < rndm_it0->taught_by.size(); i++)
                {
                    cout << rndm_it0->taught_by[i]->teacher_id << ")" << rndm_it0->taught_by[i]->teacher_name << "->" << '\t';
                    for (int j = 0; j < rndm_it0->taught_by[i]->class_preference.size(); j++)
                        cout << rndm_it0->taught_by[i]->class_preference[j]->classroom_id << ")" << rndm_it0->taught_by[i]->class_preference[j]->class_number << rndm_it0->taught_by[i]->class_preference[j]->class_section << "  ";
                    cout << '\n';
                }
            }
        }
    }
    cout << "Press enter to continue.\n";
    cin.get();
    CLASSES = classrooms.size();
    number_of_teacher = teachers.size();
    genetic_algorithm_for_time_table(subjects, teachers, classrooms);
    return 0;
}
