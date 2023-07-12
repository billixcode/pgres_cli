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


void do_insert(PGconn *conn, PGresult *res, char* line) {
    
    res = PQexec(conn, line);
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) 
        do_exit(conn, res);     
    
    PQclear(res);    
}


struct database {
  char* name[50];
  char* table[50];
  char* op[10];
  char* meta[100];
  char* sql[100];
  int fieldcnt;
};

int main(int argc, char* argv[]) {

    bool create_table = false;
    bool delete_table = false;
    bool insert_values = false;
    bool file_values = false;
    bool query = false;

    bool debug = false;

    struct database db ;
    for (int i = 1; i < argc; i++) {

        char *param_key = strtok(argv[i], "=");
        char *param_val = strtok(NULL, "=");

        if ( strcmp(param_key,"--d") == 0){
            
            strcpy(db.name, param_val);

        } else if ( strcmp(param_key,"--t") == 0){
            
            strcpy(db.table, param_val);

        } else if ( strcmp(param_key,"--s") == 0){
            query = true;
            strcpy(db.sql, param_val);

        } else if ( strcmp(param_key,"--o") == 0){
            if ( strcmp(param_val,"del") == 0){
                delete_table = true;
            }
            else if ( strcmp(param_val,"cre") == 0){
                create_table = true;
                
            }else if ( strcmp(param_val,"ins") == 0){
                insert_values = true;
                
            }
            strcpy(db.op, param_val);

        }else if ( strcmp(param_key,"--m") == 0){

            strcpy(db.meta, param_val);

        }else if ( strcmp(param_key,"--debug") == 0){
            
            debug = true;

        }          
        else{
            fprintf(stderr, "Unknown pg_cli option: %s . exiting\n", param_key );
            exit(1);
        }

    }
    if (debug){

        printf("db is %s\n", (char*) db.name);
        printf("table is %s\n", (char*)  db.table);
        printf("sql is %s\n", (char*)  db.sql);
        printf("op is %s\n", (char*)  db.op);
        printf("meta is %s\n", (char*)  db.meta);
    }

    // connection
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
    
    // drop table
    PGresult *res;
    if ( create_table || delete_table ){

        char drop_tble_cmd[1024];
        snprintf(drop_tble_cmd, 1024, "DROP TABLE IF EXISTS %s", db.table);
        if (debug){
            printf("drop table is %s\n", drop_tble_cmd);
        }
        
        res = PQexec(conn, drop_tble_cmd);
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            do_exit(conn, res);
        }
        
        PQclear(res);
    }
    
    // create table
    if (create_table){

        char create_tble_cmd[2056];
        snprintf(create_tble_cmd, 2056, "CREATE TABLE %s(%s)", db.table, db.meta);
        if (debug){
            printf("create_tble_cmd is %s\n", create_tble_cmd);
        }
        res = PQexec(conn, create_tble_cmd);
            
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            do_exit(conn, res); 
        }
        
        PQclear(res);
    
    }


    // sql insert
    if (insert_values){
        char insert_values_cmd[2056];
        snprintf(insert_values_cmd, 2056, "INSERT INTO %s VALUES(%s)", db.table, db.sql);
        if (debug){
            printf("insert_values_cmd is %s\n", insert_values_cmd);
        }
        do_insert(conn,res,insert_values_cmd);
    }

    // bulk insert
    if (file_values){

        FILE* fp;
        fp = fopen(db.sql, "r");
        if (fp == NULL) {
            perror("Failed: ");
            return 1;
        }
        int MAX_LEN = 2056;
        char buffer[MAX_LEN];
        while (fgets(buffer, MAX_LEN, fp))
        {
            // Remove trailing newline
            //buffer[strcspn(buffer, "\n")] = 0;
            if (debug){
                printf("%s\n", buffer);
            }
            
            do_insert(conn,res,buffer);
            
        }

        fclose(fp);
        return 0;

    }

    if (query){
        res = PQexec(conn, db.sql);    
    
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {

            printf("No data retrieved\n");        
            PQclear(res);
            do_exit(conn, res);
        }    
        int ncols = PQnfields(res);
        int rows = PQntuples(res);
        if (debug){
            printf("num cols %d\n", ncols);
            printf("num rows %d\n", rows);
        }
        //int string_size = 200;
        for(int i=0; i<rows; i++) {
            
            printf("| ");
            for (int y=0; y<ncols; y++) {
                
                printf("%s |", PQgetvalue(res, i, y));
                if ( y == ncols -1){
                     printf("\n");
                }
            } 
        }    
    }

    PQfinish(conn);

    return 0;
}