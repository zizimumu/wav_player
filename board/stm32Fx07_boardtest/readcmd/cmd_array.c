 int  test_function(struct cmd_tbl_s *tst_tbl,int a,int b,char *buf[])
 {
 	return 1;
 }
 
 cmd_tbl_t gCmd_array[] = {
 {"test",1,1,test_function,"just for test"}
};

