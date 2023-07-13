#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <stdbool.h>

void exit_and_close(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}


void insert(PGconn *conn, PGresult *res, char* line) {
    
    res = PQexec(conn, line);
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) 
        exit_and_close(conn, res);     
    
    PQclear(res);    
}


struct option {
    bool create_table;
    bool delete_table;
    bool insert_values;
    bool file_values;
    bool query ;
    bool debug ;
} option_default = {false, false,false,false,false,false};

struct database {
  char* name[50];
  char* table[50];
  char* op[10];
  char* meta[100];
  char* sql[100];
  int fieldcnt;
};

void print_help(){

    printf("\n");
    printf("cmnd line options:\n");
    printf("\n");
    printf("--d=$DATABASE\n");
    printf("--s=$SQL\n");
    printf("--t=$TABLE\n");
    printf("--o=$OP\n");
    printf("\n");
}

int main(int argc, char* argv[]) {

    struct database db ;
    struct option* opt = &option_default;

    for (int i = 1; i < argc; i++) {

        char *param_key = strtok(argv[i], "=");
        char *param_val = strtok(NULL, "=");

        if ( strcmp(param_key,"--d") == 0){
            
            strcpy(db.name, param_val);

        } else if ( strcmp(param_key,"--t") == 0){
            
            strcpy(db.table, param_val);

        } else if ( strcmp(param_key,"--s") == 0){

            opt->query = true;
            strcpy(db.sql, param_val);

        } else if ( strcmp(param_key,"--o") == 0){
            if ( strcmp(param_val,"del") == 0){
                opt->delete_table = true;
            }
            else if ( strcmp(param_val,"cre") == 0){
                opt->create_table = true;
                
            }else if ( strcmp(param_val,"ins") == 0){
                opt->insert_values = true;
                
            }
            strcpy(db.op, param_val);

        }else if ( strcmp(param_key,"--m") == 0){

            strcpy(db.meta, param_val);

        }else if ( strcmp(param_key,"--h") == 0){

            print_help();

        }
        else if ( strcmp(param_key,"--debug") == 0){
            opt->debug = true;

        }          
        else{
            fprintf(stderr, "Unknown pg_cli option: %s . exiting\n", param_key );
            print_help();
            exit(1);
        }

    }
    if (opt->debug){

        printf("db is %s\n", (char*) db.name);
        printf("table is %s\n", (char*)  db.table);
        printf("sql is %s\n", (char*)  db.sql);
        printf("op is %s\n", (char*)  db.op);
        printf("meta is %s\n", (char*)  db.meta);
    }

    // connection
    char connection_string[1024];
    snprintf(connection_string, 1024, "user=postgres dbname=%s", db.name);
    
    if (opt->debug){
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
    if ( opt->create_table || opt->delete_table ){

        char drop_tble_cmd[1024];
        snprintf(drop_tble_cmd, 1024, "DROP TABLE IF EXISTS %s", db.table);
        if (opt->debug){
            printf("drop table is %s\n", drop_tble_cmd);
        }
        
        res = PQexec(conn, drop_tble_cmd);
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            exit_and_close(conn, res);
        }
        
        PQclear(res);
    }
    
    // create table
    if (opt->create_table){

        char create_tble_cmd[2056];
        snprintf(create_tble_cmd, 2056, "CREATE TABLE %s(%s)", db.table, db.meta);
        if (opt->debug){
            printf("create_tble_cmd is %s\n", create_tble_cmd);
        }
        res = PQexec(conn, create_tble_cmd);
            
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            exit_and_close(conn, res); 
        }
        
        PQclear(res);
    
    }


    // sql insert
    if (opt->insert_values){
        char insert_values_cmd[2056];
        snprintf(insert_values_cmd, 2056, "INSERT INTO %s VALUES(%s)", db.table, db.sql);
        if (opt->debug){
            printf("insert_values_cmd is %s\n", insert_values_cmd);
        }
        insert(conn,res,insert_values_cmd);
    }

    // bulk insert
    if (opt->file_values){

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
            if (opt->debug){
                printf("%s\n", buffer);
            }
            
            insert(conn,res,buffer);
            
        }

        fclose(fp);
        return 0;

    }

    if (opt->query){
        res = PQexec(conn, db.sql);    
    
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {

            printf("No data retrieved\n");        
            PQclear(res);
            exit_and_close(conn, res);
        }    
        int ncols = PQnfields(res);
        int rows = PQntuples(res);
        if (opt->debug){
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