--
-- Oracle sqlldr control file for fixed width files
--
OPTIONS (ERRORS=1000, silent=feedback)
--OPTIONS (BINDSIZE=900000, SILENT=(FEEDBACK, DISCARDS), DIRECT=TRUE, ERRORS=9999, PARALLEL=TRUE)
-- UNRECOVERABLE
LOAD DATA
INFILE '/path/to/file.dat'
BADFILE '/path/to/file.bad'
DISCARDFILE '/path/to/file.dsc'
APPEND  -- or INSERT or TRUNCATE (REPLACE does a delete, so do not use it)
INTO TABLE na_source_trans_tb (
   foo POSITION(1:6) char NULLIF foo=BLANKS,
   bar POSITION(7:15) char NULLIF bar=BLANKS,
   src_id CONSTANT '100',
   seq_id RECNUM,
   acct_id position(1:6) ":src_id || '_' || :seq_id",
   first_name POSITION(22:41) char NULLIF first_name=BLANKS,
   move_dt POSITION(22:41) DATE "YYYYMMDD" NULLIF first_name=BLANKS
)

