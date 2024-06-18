// DO NOT MODIFY THE FOLLOWING SEQUENTIAL FUNCTION
void seq_convert (int *p, struct attrib im) {            
    int cnt = 0, ch[3] = {0};     
    for(int i = 0; i < im.no_rows; i++) {
        for(int j = 0; j < im.no_cols; j++){
            for(int k = 0; k < 3; k++){
                ch[k] = p[cnt];
                cnt++; 
            }
            p[cnt-1] = (ch[0] + ch[1])/2;
            p[cnt-2] = (ch[0] + ch[2])/2;
            p[cnt-3] = (ch[1] + ch[2])/2;
        }
    }
}

// UNCOMMENT FUNCTION par_convert FIRST AND THEN
// TODO: COMPLETE THE PARALLEL PROCESSING FUNCTION BASED ON
// THE SERIAL PROCESSING GIVEN IN FUNCTION seq_convert

void par_convert (int *p, struct attrib im) {
   int cnt = 0, ch[3] = {0};   
   #pragma omp parallel for reduction(+:cnt)  
    for(int i = 0; i < im.no_rows; i++) {
        for(int j = 0; j < im.no_cols; j++){
            for(int k = 0; k < 3; k++){
                ch[k] = p[cnt];
                cnt++; 
            }
        }
    }
    p[cnt-1] = (ch[0] + ch[1])/2;
    p[cnt-2] = (ch[0] + ch[2])/2;
    p[cnt-3] = (ch[1] + ch[2])/2;

    
}




