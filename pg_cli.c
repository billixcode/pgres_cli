#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 

void exit_and_close(PGconn *conn, PGresult *res) {
    
    fprintf(stderr, "%s\n", PQerrorMessage(conn));    

    PQclear(res);
    PQfinish(conn);    
    
    exit(1);
}


void insert(PGconn *conn, PGresult *res, char* line) {
    
    res = PQexec(conn, line);
    //free(conn);
    if (PQresultStatus(res) != PGRES_COMMAND_OK){
        exit_and_close(conn, res);     
    }else{
        PQclear(res);  
    }  
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
    printf("cmd line options:\n");
    printf("\n");
    printf("-d $DATABASE\n");
    printf("-s $SQL\n");
    printf("-t $TABLE\n");
    printf("-o $OP\n");
    printf("\n");
}


int main(int argc, char *argv[]) 
{
    int opt;
    extern char *optarg;
    extern int optind, optopt;
    struct database db ;
    struct option* opt_flg = &option_default;
      
    while((opt = getopt(argc, argv, "d:t:s:o:m:hv")) != -1) 
    { 
        switch(opt) 
        { 
            
            case 'd': 
                strcpy(db.name, optarg);
                break; 

            case 't': 
                strcpy(db.table, optarg);
                break; 

            case 's': 
                opt_flg->query = true;
                strcpy(db.sql, optarg);
                break; 
            
            case 'o': 
                if ( strcmp(optarg,"del") == 0){
                    opt_flg->delete_table = true;
                }
                else if ( strcmp(optarg,"cre") == 0){
                    opt_flg->create_table = true;
                    
                }else if ( strcmp(optarg,"ins") == 0){
                    opt_flg->insert_values = true;
                }
                else if ( strcmp(optarg,"fle") == 0){
                    opt_flg->file_values = true;
                }
                strcpy(db.op, optarg);
                break; 
            
            case 'm': 
                strcpy(db.meta, optarg);
                break; 

            case 'h': 
                print_help();
                exit(1);
            
            case 'v':
                opt_flg->debug = true;
                break;

            default:
                fprintf(stderr, "Unknown option: %s . exiting\n", opt );
                print_help();
                exit(1);
        } 
    } 
      

    execute(opt_flg,db);

    return 0;
}

void execute(struct option *opt_flg, struct database db){

    if (opt_flg->debug){

        printf("db is %s\n", (char*) db.name);
        printf("table is %s\n", (char*)  db.table);
        printf("sql is %s\n", (char*)  db.sql);
        printf("op is %s\n", (char*)  db.op);
        printf("meta is %s\n", (char*)  db.meta);
    }

    // connection
    char connection_string[1024];
    snprintf(connection_string, 1024, "user=postgres dbname=%s", db.name);
    
    if (opt_flg->debug){
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
    if ( opt_flg->create_table || opt_flg->delete_table ){

        char drop_tble_cmd[1024];
        snprintf(drop_tble_cmd, 1024, "DROP TABLE IF EXISTS %s", db.table);
        if (opt_flg->debug){
            printf("drop table is %s\n", drop_tble_cmd);
        }
        
        res = PQexec(conn, drop_tble_cmd);
        
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            exit_and_close(conn, res);
        }
        
        PQclear(res);
    }
    
    // create table
    if (opt_flg->create_table){

        char create_tble_cmd[2056];
        snprintf(create_tble_cmd, 2056, "CREATE TABLE %s(%s)", db.table, db.meta);
        if (opt_flg->debug){
            printf("create_tble_cmd is %s\n", create_tble_cmd);
        }
        res = PQexec(conn, create_tble_cmd);
            
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            exit_and_close(conn, res); 
        }
        
        PQclear(res);
    
    }


    // sql insert
    if (opt_flg->insert_values){
        char insert_values_cmd[2056];
        snprintf(insert_values_cmd, 2056, "INSERT INTO %s VALUES ( %s ) RETURNING *", db.table, db.sql);
        if (opt_flg->debug){
            printf("insert_values_cmd is %s\n", insert_values_cmd);
        }
        insert(conn,res,insert_values_cmd);
    }

    // bulk insert
    if (opt_flg->file_values){

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
            if (opt_flg->debug){
                printf("%s\n", buffer);
            }
            
            insert(conn,res,buffer);
            
        }

        fclose(fp);
        return 0;

    }

    if (opt_flg->query){
        res = PQexec(conn, db.sql);    
    
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {

            printf("No data retrieved\n");        
            PQclear(res);
            exit_and_close(conn, res);
        }    
        int ncols = PQnfields(res);
        int rows = PQntuples(res);
        if (opt_flg->debug){
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

}
