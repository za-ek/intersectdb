# intersectdb
Database for storing intersections

# Preface
This database is allow you to store a count of intersections within a set of items.

# Install
On the project root directory
- `make clean && make`

# Run
On the project root directory
- `./intersectdb [PORT_NO = 2450]`

# Available commands

You can send commands connecting by `netcat` for example `netcat 127.0.0.1 2450`. Any response returns with a string "END".

- Create a database
`CREATE DB2 %db_name% %elements_count%`

- Increment value of intersection between X and Y
`INC %db_name% X Y [%inc_value% = 1]`

- Get value of intersection between X and Y
`READ %db_name% X Y`

- Get value of all intersections with X
`FETCH %db_name% X`
