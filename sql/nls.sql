select rtrim(a)||'_'||rtrim(b)||'.'||rtrim(c) NLS_LANG from
(select value a from sys.nls_database_parameters
where parameter='NLS_LANGUAGE'),
(select value b from sys.nls_database_parameters
where parameter='NLS_TERRITORY'),
(select value c from sys.nls_database_parameters
where parameter='NLS_CHARACTERSET')
/
