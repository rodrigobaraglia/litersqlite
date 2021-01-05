#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    static char line[256];
    char *ptr;
    unsigned int len;

    if (lineptr == NULL || n == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    if (ferror(stream))
    {
        return -1;
    }

    if (feof(stream))
    {
        return -1;
    }

    fgets(line, 256, stream);

    ptr = strchr(line, '\n');

    if (ptr)
    {
        *ptr = '\0';
    }

    len = strlen(line);

    if ((len + 1) < 256)
    {
        ptr = realloc(*lineptr, 256);
        if (ptr == NULL)
        {
            return -1;
        }
        *lineptr = ptr;
        *n = 256;
    }
    strcpy(*lineptr, line);
    return len;
}

#endif

typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCES,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct
{
    StatementType type;
    Row row_to_insert;
} Statement;

typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;

} InputBuffer;

InputBuffer *new_input_buffer()
{
    InputBuffer *ib = (InputBuffer *)malloc(sizeof(InputBuffer));
    ib->buffer = NULL;
    ib->buffer_length = 0;
    ib->buffer_length = 0;
    return ib;
}

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

void serialize_row(Row *source, void *destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void print_prompt()
{
    printf("db > ");
}

void read_input(InputBuffer *ib)
{
    ssize_t bytes_read = getline(&(ib->buffer), &(ib->buffer_length), stdin);

    ib->input_length = bytes_read;
    ib->buffer[bytes_read] = 0;
}

void close_input_buffer(InputBuffer *ib)
{
    free(ib->buffer);
    free(ib);
}

MetaCommandResult do_meta_command(InputBuffer *ib)
{
    if (strcmp(ib->buffer, ".exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

PrepareResult prepare_statement(InputBuffer *ib, Statement *stmt)
{
    if (strncasecmp(ib->buffer, "insert", 6) == 0)
    {
        stmt->type = STATEMENT_INSERT;

        ////***To add later***
        // int args_assigned = sscanf(
        //     ib->buffer,
        //     "insert %d %s %s",
        //     &(stmt->row_to_insert.id),
        //     stmt->row_to_insert.username,
        //     stmt->row_to_insert.email);

        // if (args_assigned < 3)
        // {
        //     return PARSE_SYNTAX_ERROR;
        // }

        return PREPARE_SUCCES;
    }
    if (strcasecmp(ib->buffer, "select") == 0)
    {
        stmt->type = STATEMENT_SELECT;
        return PREPARE_SUCCES;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement *stmt)
{
    switch (stmt->type)
    {
    case (STATEMENT_INSERT):
        printf("Here goes the INSERT\n");
        break;
    case (STATEMENT_SELECT):
        printf("Here goes the SELECT\n");
        break;
    }
}

int main(int argc, char *argv[])
{
    InputBuffer *ib = new_input_buffer();

    while (true)
    {
        print_prompt();
        read_input(ib);

        if (ib->buffer[0] == '.')
        {
            switch (do_meta_command(ib))
            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("Unrecognized command '%s'\n", ib->buffer);
                continue;
            }
        }

        Statement stmt;
        switch (prepare_statement(ib, &stmt))
        {
        case (PREPARE_SUCCES):
            break;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            if (ib->input_length > 0)
            {
                printf("Unrecognized keyword at start of '%s'\n", ib->buffer);
            }
            continue;
        }

        execute_statement(&stmt);
        printf("Executed.\n");
    }
}
