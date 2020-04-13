OPTIONS (ERRORS=1000, DIRECT=TRUE)
LOAD DATA
INFILE 'dni_age_ranges.csv'
BADFILE 'dni_age_ranges.bad'
DISCARDFILE 'dni_age_ranges.disc'
REPLACE
into table dni_age_ranges
fields terminated by ','
trailing nullcols
(
  age_range NULLIF age_range=BLANKS,
  dni_upper NULLIF dni_upper=BLANKS, 
  dni_lower NULLIF dni_lower=BLANKS, 
  year_upper NULLIF year_upper=BLANKS, 
  year_lower NULLIF year_upper=BLANKS, 
  age_code NULLIF age_code=BLANKS
)

