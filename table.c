#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <stdbool.h>

void do_exit(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}

struct field{
    char name[50];
    char type[50];
    char modifier[50];
};

struct database {
  char* name[50];
  char* table[50];
  char* op[10];
  char* sql[100];
  struct field fields[1];
  int fieldcnt;
};

int main(int argc, char* argv[]) {

    bool create_table = false;
    bool delete_table = false;
    bool debug = false;

    struct database db ;
    for (int i = 1; i < argc; i++) {

        char *token_1 = strtok(argv[i], ":"); 


        if ( strcmp(token_1,"db") == 0){
            
            char *token_2 = strtok(NULL, ":"); 
            char *token_3 = strtok(NULL, ":"); 
            char *token_4 = strtok(NULL, ":"); 
            char *token_5 = strtok(NULL, ":"); 
            strcpy(db.name, token_2);
            strcpy(db.table, token_3);
            strcpy(db.op, token_4);
            if (strcmp(db.op,"cr") == 0){
                create_table = true;
                strcpy(db.sql, token_5);
            }else if (strcmp(db.op,"de") == 0){
                delete_table  = true;
            }else{
                fprintf(stderr, "Unknown table option: %s . exiting\n", db.op);
                exit(1);
            }

            
        }else if ( strcmp(token_1,"debug") == 0){
            debug = true;
        }

        

    }
    if (debug){
        printf("db is %s\n", (char*) db.name);
        printf("table is %s\n", (char*)  db.table);
        printf("sql is %s\n", (char*)  db.sql);
    }
    char connection_string[1024];
    snprintf(connection_string, 1024, "user=postgres dbname=%s", db.name);
    
    if (debug){
       printf("conn is %s\n", connection_string);
    }
    PGconn *conn = PQconnectdb(connection_string);

    if (PQstatus(conn) == CONNECTION_BAD) {
        
        fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
            
        PQfinish(conn);
        exit(1);
    }
    
    PGresult *res;
    if ( create_table || delete_table ){

        char drop_tble_cmd[1024];
        snprintf(drop_tble_cmd, 1024, "DROP TABLE IF EXISTS %s", db.table);
        printf("drop table is %s\n", drop_tble_cmd);
        res = PQexec(conn, drop_tble_cmd);
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            do_exit(conn, res);
        }
        
        PQclear(res);
    }
    
    if (create_table){

        char create_tble_cmd[2056];
        snprintf(create_tble_cmd, 2056, "CREATE TABLE %s(%s)", db.table, db.sql);
        printf("create_tble_cmd is %s\n", create_tble_cmd);
        res = PQexec(conn, create_tble_cmd);
            
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            do_exit(conn, res); 
        }
        
        PQclear(res);
    
    }
    PQfinish(conn);

    return 0;
}