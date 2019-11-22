#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "tgamepad.h"

#define ROW 20
#define COL 20

enum elem_type
{
    EMPTY_BLOCK,
    WALL_BLOCK,
    SNAKE_BLOCK,
    FOOD_BLOCK
};
enum moving_type
{
    MOVING_LEFT,
    MOVING_RIGHT,
    MOVING_UP,
    MOVING_DOWN
};
enum moving_status
{
    STATIC_STATUS,
    MOVING_STATUS
};
enum game_mode
{
    CLASSIC_MODE,
    CAMPAIGN_MODE
};
enum game_status
{
    PLAYING_STATUS,
    PAUSE_STAUS,
    DIED_STATUS
};

struct snake
{
    uint8_t x;
    uint8_t y;
    struct snake *next;
};

uint8_t map[ROW][COL];
uint8_t snake_moving_type = MOVING_RIGHT;
uint8_t snake_moving_status = STATIC_STATUS;
uint8_t snake_moving_speed = 1;

uint8_t snake_game_mode = CLASSIC_MODE;
uint8_t snake_game_status = PLAYING_STATUS;
uint8_t snake_game_life = 2;

uint16_t snake_game_point = 0;
uint16_t snake_wall_counter = 0;
uint16_t snake_lengh = 2;

struct snake *snake_head = NULL;
pthread_mutex_t mutex;

void gotoxy(int x, int y)
{
    printf("\033[%d;%df", x, y);
}

void printf_row_buff(int row_size)
{
    int buff_len;
    buff_len = (int)((row_size - ROW) / 2);
    if (0 < buff_len)
    {
        while (buff_len > 0)
        {
            buff_len--;
            printf("\n");
        }
    }
    return;
}

void printf_col_buff(int col_size)
{
    int buff_len;
    buff_len = (int)((col_size - 2 * COL) / 2);
    if (0 < buff_len)
    {
        while (buff_len > 0)
        {
            buff_len--;
            printf(" ");
        }
    }
    return;
}

void food_gen()
{
    uint8_t i, j;
    srand(time(0));
    uint16_t food_position = (uint16_t)rand();
    uint16_t count = 0;
    uint16_t rest_slot = (ROW * COL) - snake_wall_counter - snake_lengh - 1;
    food_position = food_position % rest_slot;
    for (i = 0; i < COL; i++)
    {
        for (j = 0; j < ROW; j++)
        {
            if (EMPTY_BLOCK == map[i][j])
            {
                if (food_position == count)
                {
                    map[i][j] = FOOD_BLOCK;
                    return;
                }
                count++;
            }
        }
    }
    return;
}

void map_reset()
{
    int i, j;
    for (i = 0; i < ROW; i++)
    {
        for (j = 0; j < COL; j++)
        {
            map[i][j] = EMPTY_BLOCK;
        }
    }
    return;
}

void map_0_gen()
{
    map_reset();
    food_gen();
    snake_wall_counter = 0;
    return;
}

void map_1_gen()
{
    int i;
    for (i = 0; i < COL; i++)
    {
        map[0][i] = 1;
        map[ROW - 1][i] = 1;
    }
    for (i = 0; i < ROW; i++)
    {
        map[i][0] = 1;
        map[i][COL - 1] = 1;
    }

    food_gen();
    snake_wall_counter = 2 * (ROW + COL - 2);
    return;
}

void map_2_gen()
{
    int i;
    for (i = 0; i < COL; i++)
    {
        map[0][i] = 1;
        map[ROW - 1][i] = 1;
    }
    for (i = 0; i < ROW; i++)
    {
        map[i][0] = 1;
        map[i][COL - 1] = 1;
    }

    for (i = 10; i < COL - 10; i++)
    {
        map[10][i] = 1;
        map[ROW - 10][i] = 1;
    }

    food_gen();
    snake_wall_counter = 2 * (ROW + COL - 2) + 2 * (ROW - 20);
    return;
}

void map_display()
{
    printf("\033[H\033[J");
    int i, j;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    printf_row_buff((int)w.ws_row);
    for (i = 0; i < ROW; i++)
    {
        printf_col_buff((int)w.ws_col);
        for (j = 0; j < COL; j++)
        {
            if (EMPTY_BLOCK == map[i][j])
                printf("  ");
            else if (FOOD_BLOCK == map[i][j])
                printf("[]");
            else
                printf("\u2588\u2588");
        }
        printf("\r\n");
    }
    printf_col_buff((int)w.ws_col);
    printf("POINT: %d \tLENGTH: %d \tLIFE: %d ", snake_game_point, snake_lengh, snake_game_life);
    return;
}

void map_display_beta()
{
    int i, j;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    for (i = 0; i < ROW; i++)
    {
        for (j = 0; j < COL; j++)
        {
            if (EMPTY_BLOCK == map[i][j])
            {
                gotoxy(i + 1, 2 * j + 1);
                printf(" ");
                gotoxy(i + 1, 2 * j + 2);
                printf(" ");
            }
            else if (FOOD_BLOCK == map[i][j])
            {
                gotoxy(i + 1, 2 * j + 1);
                printf("[");
                gotoxy(i + 1, 2 * j + 2);
                printf("]");
            }
            else
            {
                gotoxy(i + 1, 2 * j + 1);
                printf("\u2588");
                gotoxy(i + 1, 2 * j + 2);
                printf("\u2588");
            }
        }
    }
    gotoxy(ROW + 1, 1);
    printf("POINT: %d \tLENGTH: %d \tLIFE: %d ", snake_game_point, snake_lengh, snake_game_life);
    return;
}

struct snake *snake_create(uint8_t x, uint8_t y)
{
    struct snake *tmp;
    tmp = (struct snake *)malloc(sizeof(struct snake));
    tmp->x = x;
    tmp->y = y;
    tmp->next = NULL;
    return tmp;
}

void snake_printf(struct snake *head)
{
    while (NULL != head)
    {
        printf("[%d][%d] %p\n", head->x, head->y, head->next);
        head = head->next;
    }
}

struct snake *snake_push(struct snake *head, uint8_t x, uint8_t y)
{
    struct snake *tmp = snake_create(x, y);
    if (NULL != head)
    {
        tmp->next = head;
    }
    return tmp;
}

void snake_tail_delete(struct snake *head)
{
    if (NULL == head)
        return;
    else if (NULL == head->next)
        return;
    else
    {
        struct snake *prev = head;
        struct snake *tail = prev->next;
        while (NULL != tail->next)
        {
            prev = prev->next;
            tail = tail->next;
        }
        map[tail->x][tail->y] = EMPTY_BLOCK;
        free(tail);
        prev->next = NULL;
    }
    return;
}

struct snake *snake_tail_get(struct snake *head)
{
    if (NULL == head)
        return NULL;
    struct snake *tmp = head;
    while (NULL != tmp->next)
    {
        tmp = tmp->next;
    }
    return tmp;
}

void snake_lengh_update(struct snake *head)
{
    uint16_t count = 0;
    if (NULL == head)
        snake_lengh = 0;
    else
    {
        while (NULL != head)
        {
            head = head->next;
            count++;
        }
        snake_lengh = count;
    }
    return;
}

struct snake *snake_delete_after_node(struct snake *head, struct snake *current)
{
    if (NULL == current)
        return head;
    else
    {
        struct snake *next;
        next = current->next;
        current->next = NULL;
        current = next;
        while (NULL != current)
        {
            next = current->next;
            map[current->x][current->y] = EMPTY_BLOCK;
            free(current);
            current = next;
        }
    }
    return head;
}

void snake_delete_after_value(struct snake *head, uint8_t x, uint8_t y)
{
    struct snake *tmp = head;
    if (NULL == head)
        return;
    else
    {
        while (NULL != tmp)
        {
            if ((tmp->x) == x && (tmp->y) == y)
            {
                snake_delete_after_node(head, tmp);
                return;
            }
            tmp = tmp->next;
        }
        return;
    }
}

struct snake *snake_delete_all(struct snake *head)
{
    struct snake *next;
    if (NULL == head)
        return NULL;
    else
    {
        while (NULL != head)
        {
            next = head->next;
            map[head->x][head->y] = EMPTY_BLOCK;
            free(head);
            head = next;
        }
        return NULL;
    }
}

struct snake *snake_new_game(struct snake *head)
{
    snake_game_point = 0;
    snake_lengh = 2;
    snake_moving_type = MOVING_RIGHT;
    snake_game_status = PLAYING_STATUS;

    head = snake_push(head, 5, 5);
    head = snake_push(head, 5, 6);
    return head;
}

struct snake *snake_moving(struct snake *head, uint8_t type)
{
    if (NULL == head)
        return NULL;
    int x = (int)head->x;
    int y = (int)head->y;

    if (MOVING_LEFT == type)
        y--;
    else if (MOVING_RIGHT == type)
        y++;
    else if (MOVING_UP == type)
        x--;
    else if (MOVING_DOWN == type)
        x++;

    if (y < 0)
        y = COL - 1;
    else if (y > (COL - 1))
        y = 0;
    if (x < 0)
        x = ROW - 1;
    else if (x > (ROW - 1))
        x = 0;

    struct snake *tmp;
    if (EMPTY_BLOCK == map[x][y])
    {
        map[x][y] = SNAKE_BLOCK;
        snake_tail_delete(head);
        tmp = snake_push(head, (uint8_t)x, (uint8_t)y);
    }
    else if (FOOD_BLOCK == map[x][y])
    {
        snake_game_point++;
        snake_lengh++;

        map[x][y] = SNAKE_BLOCK;
        tmp = snake_push(head, (uint8_t)x, (uint8_t)y);
        food_gen();
    }
    else if (WALL_BLOCK == map[x][y])
    {
        sleep(1);
        tmp = snake_delete_all(head);
        snake_game_status = DIED_STATUS;
    }
    else if (SNAKE_BLOCK == map[x][y])
    {
        snake_delete_after_value(head, (uint8_t)x, (uint8_t)y);
        snake_tail_delete(head);
        snake_lengh_update(head);

        map[x][y] = SNAKE_BLOCK;
        snake_tail_delete(head);
        tmp = snake_push(head, (uint8_t)x, (uint8_t)y);
    }
    snake_moving_status = MOVING_STATUS;
    return tmp;
}

void game_display_pick_level()
{
    printf("\033[H\033[J");
    printf("WHO ARE YOU, MY FRIEND?\n\n\n");
    printf("0. I'm a chicken\n");
    printf("1. I'm a fresh engineer\n");
    printf("2. I'm a junior engineer\n");
    printf("3. I'm a senior engineer\n");
    printf("4. I'm a expert engineer\n");
    printf("5. I'm a master\n");
    printf("6. I am Iron man\n");
    char input = '1';
    // scanf(" %c", &input);
    printf("\033[H\033[J");
    switch (input)
    {
    case '0':
        printf("You should go home, chicken\n");
        sleep(1);
        exit(EXIT_SUCCESS);
        break;
    case '1':
        snake_moving_speed = 2;
        break;
    case '2':
        snake_moving_speed = 4;
        break;
    case '3':
        snake_moving_speed = 6;
        break;
    case '4':
        snake_moving_speed = 8;
        break;
    case '5':
        printf("What are u doing here ???\n");
        sleep(2);
        snake_moving_speed = 10;
        break;
    case '6':
        printf("You have my respect Stark\n");
        sleep(2);
        snake_moving_speed = 16;
        break;
    default:
        printf("Go home bro, We Don't Belong Together\n");
        sleep(2);
        exit(EXIT_SUCCESS);
        break;
    }
}

void game_display_pick_map()
{
    printf("\033[H\033[J");
    printf("PICK MAP [0] [1] [2] [3] [4]?\n");
    char input = 0;
    // scanf(" %c", &input);
    map_reset();

    switch (input)
    {
    case '0':
        map_0_gen();
        break;
    case '1':
        map_1_gen();
        break;
    case '2':
        map_2_gen();
        break;
    case '3':
        map_2_gen();
        break;
    case '4':
        map_2_gen();
        break;
    default:
        map_2_gen();
        break;
    }
}

void game_display_gameover()
{
    printf("\033[H\033[J");
    printf("GAME OVER\n");
    printf("GOOD LUCK\n");
    sleep(1);
}

void *display(void *arg)
{
    while (DIED_STATUS != snake_game_status)
    {
        pthread_mutex_lock(&mutex);
        map_display();
        pthread_mutex_unlock(&mutex);
        if (PAUSE_STAUS == snake_game_status)
        {
            printf("\tPAUSE");
        }
        printf("\n");
        usleep(50000);
    }
    printf("\033[H\033[J");
    printf("YOU DIED\n");
    printf("Press any key to continue\n");
    return NULL;
}

void *backend(void *arg)
{
    while (DIED_STATUS != snake_game_status)
    {
        uint32_t sleep_time = 250000;
        if (PLAYING_STATUS == snake_game_status)
        {
            pthread_mutex_lock(&mutex);
            snake_head = snake_moving(snake_head, snake_moving_type);
            pthread_mutex_unlock(&mutex);
        }
        sleep_time = sleep_time / snake_moving_speed;
        usleep(sleep_time);
    }
    snake_head = snake_delete_all(snake_head);
    return NULL;
}

void *control(void *arg)
{
    // static struct termios oldt, newt;
    while (DIED_STATUS != snake_game_status)
    {

        // tcgetattr(STDIN_FILENO, &oldt);
        // newt = oldt;
        // newt.c_lflag &= ~(ICANON);
        // tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        char input = tgamepad_read();
        if (STATIC_STATUS != snake_moving_status)
        {
            if ('a' == input && (MOVING_RIGHT != snake_moving_type))
                snake_moving_type = MOVING_LEFT;
            else if ('s' == input && (MOVING_UP != snake_moving_type))
                snake_moving_type = MOVING_DOWN;
            else if ('d' == input && (MOVING_LEFT != snake_moving_type))
                snake_moving_type = MOVING_RIGHT;
            else if ('w' == input && (MOVING_DOWN != snake_moving_type))
                snake_moving_type = MOVING_UP;

            snake_moving_status = STATIC_STATUS;
            // usleep(100000);
        }
        if ('p' == input && (PLAYING_STATUS == snake_game_status))
            snake_game_status = PAUSE_STAUS;
        else if ('p' == input && (PAUSE_STAUS == snake_game_status))
            snake_game_status = PLAYING_STATUS;
        else if ('q' == input && (PAUSE_STAUS == snake_game_status))
        {
            snake_game_status = DIED_STATUS;
            snake_game_life = 0;
        }
        // tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
    return NULL;
}

int main(void)
{
    uint8_t i;
    pthread_mutex_init(&mutex, NULL);
    pthread_t thread_id[3];

    while (1)
    {
        game_display_pick_level();
        game_display_pick_map();
        snake_game_life = 3;
        while (snake_game_life > 0)
        {
            snake_head = snake_new_game(snake_head);
            snake_game_life--;

            pthread_mutex_init(&mutex, NULL);
            pthread_create(&thread_id[0], NULL, backend, NULL);
            pthread_create(&thread_id[1], NULL, control, NULL);
            pthread_create(&thread_id[2], NULL, display, NULL);
            for (i = 0; i < 3; i++)
            {
                pthread_join(thread_id[i], NULL);
            }
            pthread_mutex_destroy(&mutex);
        }
        game_display_gameover();
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}