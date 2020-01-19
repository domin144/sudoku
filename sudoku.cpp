/* Dominik Wójt */
#include <cstdio>

// types -----------------------------------------------------------------------
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned long int ulong;

struct area
{
    /* 9-bit field shifted by 1 to the left, 0-presense, 1-absence of digit */
    int digits;
    /* 9-bit field, 0-occupied, 1-empty, actually needed only to find and sort
     * vacancies. If one really tried really hard one could reduce to only one
     * member per area. */
    int gaps;
};

struct vacancy
{
    /* values calculated at the beginning */
    int position;
    /* in which area it is contained, number 0..26 */
    int in_row, in_column, in_square;
    /* and at which position, looks unneeded(if gaps are unneded in searching)*/
    // int zpzoff, zpnoff, zploff;

    /* values calculated when putting on a stack */
    /* 1..9-filled with a digit, 0-original state
     * (remeber to rever when going back) */
    int value;
    /* as above but as mask with 0 at beginning */
    int digit;
    /* values to revert to, they look unneeded */
    // int bpoziom, int bpion, int bpole;
};
// types -----------------------------------------------------------------------

// constants -------------------------------------------------------------------
enum
{
    ROW,
    COLUMN,
    SQUARE
};
// constants -------------------------------------------------------------------

// variables -------------------------------------------------------------------
FILE* file;
int i, j;
int z;

int puzzle[81];
int position;
area areas[27]; // 0..8 - rows, 9..17 - columns, 18..26 - squares
vacancy vacancies[9 * 9]; // 9*9 is a maximal value
int w;
int limit = 5; // limit on the number of solutions
// variables -------------------------------------------------------------------

// functions -------------------------------------------------------------------
int read(); // finished, lighly tested
int append(); // finished, lighly tested
int count(); // finished, lighly tested
int sort_vacancies(); // finished, lighly tested
int search(); // finished, to be tested
int sum_of_ones(int); // finished, to be tested
int give_position(int, int); // finished, to be tested
// functions -------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (read())
        return 1;
    file = fopen("solution.txt", "wt");
    if (!file)
        return 1;

    if (count())
    {
        fprintf(file, "no solutions, because of invalid data\n");
        /* no solutions */
        return 0;
    }
    sort_vacancies();
    if (search())
        fprintf(file, "solutions limit reached\n");
    else
        fprintf(file, "no more solutions\n");

    //	append();
    fclose(file);
    return 0;
}

// function definitions---------------------------------------------------------
int read()
{
    char pz;
    file = fopen("plansza.txt", "rt");
    if (!file)
        return 1;

    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            do
                fscanf(file, "%c", &pz);
            while ((pz & 0xf0) != 0x30 && pz != 'x');
            if (pz != 'x')
                pz &= 0xf;
            else
                pz = 0;
            puzzle[i * 9 + j] = pz;
        }
    }

    fclose(file);
    return 0;
}
//------------------------------------------------------------------------------
int append()
{
    for (i = 0; i < 9; i++)
    {
        fprintf(
            file,
            "%d %d %d  %d %d %d  %d %d %d\n",
            puzzle[i * 9],
            puzzle[i * 9 + 1],
            puzzle[i * 9 + 2],
            puzzle[i * 9 + 3],
            puzzle[i * 9 + 4],
            puzzle[i * 9 + 5],
            puzzle[i * 9 + 6],
            puzzle[i * 9 + 7],
            puzzle[i * 9 + 8]);
        if ((i + 1) % 3 == 0)
            fprintf(file, "\n");
    }
    fprintf(file, "\n");
    limit--;
    if (limit)
        return 0; // limit not reached yet
    else
        return 1; // limit reached
}
//------------------------------------------------------------------------------
int count()
{
    for (i = 0; i < 27; i++)
    {
        areas[i].digits = 0x3fe; // 9 ones and a zero
        areas[i].gaps = 0x1ff; // 9 ones
    }
    for (i = 0; i < 9; i++)
        for (j = 0; j < 9; j++)
            if ((z = puzzle[i * 9 + j]))
            {
                z = 0x1 << z;
                if (areas[ROW * 9 + i].digits & z) // if the digit is not yet there
                    areas[ROW * 9 + i].digits &= ~z; // zero'ing at 0x1<<digit
                else
                    return 1; // the digit is already there, invalid data
                if (areas[COLUMN * 9 + j].digits & z)
                    areas[COLUMN * 9 + j].digits &= ~z;
                else
                    return 1;
                if (areas[SQUARE * 9 + i / 3 * 3 + j / 3].digits & z)
                    areas[SQUARE * 9 + i / 3 * 3 + j / 3].digits &= ~z;
                else
                    return 1;
                areas[ROW * 9 + i].gaps &= ~(0x1 << j);
                areas[COLUMN * 9 + j].gaps &= ~(0x1 << i);
                areas[SQUARE * 9 + i / 3 * 3 + j / 3].gaps &=
                    ~(0x1 << (i % 3 * 3 + j % 3));
            }
    return 0;
}

//------------------------------------------------------------------------------
int sum_of_ones(int a)
{
    int sum = 0;
    do
        if (a & 0x1)
            sum++;
    while (a >>= 1);
    return sum;
}
//------------------------------------------------------------------------------
int sort_vacancies()
{
    int b;
    int min;
    int sums[27]; // number of gaps
    int order[27];
    for (i = 0; i < 27; i++)
    {
        sums[i] = sum_of_ones(areas[i].gaps);
        order[i] = i;
    }

    b = 26;
    w = 0;
    while (b)
    {
        min = 0;
        for (i = 1; i <= b; i++)
            if (sums[order[i]] < sums[order[min]])
                min = i;
        if (min != b)
        {
            z = order[min];
            order[min] = order[b];
            order[b] = z;

            // order[b] - the most filled area, the least gaps
            for (j = 0, z = 0x1; j < 9; j++, z <<= 1) // for every position
                if (areas[order[b]].gaps & z) // if there is a gap
                {
                    position = give_position(order[b], j);
                    /* fill the position of the vacancy */
                    vacancies[w].position = position;

                    vacancies[w].in_row = ROW * 9 + position / 9;
                    //					vacancies[w].zpzoff =
                    //0x1<<(position%9);
                    vacancies[w].in_column = COLUMN * 9 + position % 9;
                    //					vacancies[w].zpnoff =
                    //0x1<<(position/9);
                    vacancies[w].in_square =
                        SQUARE * 9 + position / 27 * 3 + position % 9 / 3;
                    //					vacancies[w].zploff =
                    //0x1<<(position/9*3+position%3);

                    areas[vacancies[w].in_row].gaps &= ~(0x1 << (position % 9));
                    sums[vacancies[w].in_row]++;
                    areas[vacancies[w].in_column].gaps &=
                        ~(0x1 << (position / 9));
                    sums[vacancies[w].in_column]++;
                    areas[vacancies[w].in_square].gaps &=
                        ~(0x1 << (position / 9 % 3 * 3 + position % 3));
                    sums[vacancies[w].in_square]++;

                    w++;
                }
        }
        b--;
    }
    // w - number of vacancies
    // vacancies[] - table of sorted vacancies (in order from strategic to
    // less important)

    return 0;
}
//------------------------------------------------------------------------------
int search()
{
    int g;
    vacancy* wk;

    for (i = 0; i < w; i++) // zero'ing of the values
        vacancies[i].value = 0, vacancies[i].digit = 0x1;

    g = 0; // glebokosc
    while (g >= 0)
    {
        wk = &vacancies[g]; // investigating vacancy g
        do
            wk->value++, wk->digit <<= 1;
        while (!(wk->digit & areas[wk->in_row].digits
                 & areas[wk->in_column].digits & areas[wk->in_square].digits)
               && wk->value < 10);
        /* we found (or not-10) first(or another) digit viable for insertion */
        if (wk->value == 10)
        {
            /* STOP - no solutions on this path, going back */

            wk->value = 0;
            wk->digit = 0x1;
            // puzzle[wk->position] = 0; // most probably too carefull
            g--;
            if (g >= 0)
            {
                wk = &vacancies[g];
                areas[wk->in_row].digits |=
                    wk->digit; // tidy up(set to 1)
                areas[wk->in_column].digits |= wk->digit;
                areas[wk->in_square].digits |= wk->digit;
            }
        }
        else
        {
            puzzle[wk->position] = wk->value;
            if (g == w - 1) // STOP - solution found
            {
                if (append()) // limit was reached, in the other case
                              // junst continue
                    return 1;
            }
            else //(backup - spurious) change of digits in areas, go deeper
            {
                areas[wk->in_row].digits &= ~wk->digit;
                areas[wk->in_column].digits &= ~wk->digit;
                areas[wk->in_square].digits &= ~wk->digit;
                g++;
            }
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
int give_position(int str, int offset)
{
    switch (str / 9)
    {
    case ROW:
        return str * 9 + offset; // str%9 = str
    case COLUMN:
        return offset * 9 + str - 9; // str%9 = str-9
    case SQUARE:
        return (str - 18) / 3 * 27 + offset / 3 * 9 + str % 3 * 3
            + offset % 3; // str%9 = str-18
    }
    return 81;
}
