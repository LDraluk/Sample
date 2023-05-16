# Leah Draluk Submission
Put attached .cpp and .h files into some dir and cd to this dir
From within this dir clone Current repo: git clone https://github.com/c5t/Current
Create a symlink to Current standard Makefile: ln -s Current/scripts/Makefile
Modify Current/scripts/MakefileImpl   to add -lsqlite3 to LDFLAGS   - to make sqlite db C++ api to link correctly
Run make main
Start http server that accepts csv files and user name 
      ./main
Create config file (see below how config file is build)  - example config file leah is attached 
Submit request to server  curl -X POST -d '{"author": "Leah", "csv":"Leah,Draluk  Vadim,Draluk"}' http://localhost:12345/post_csv 
   ( request is in JSON format and have csv upload and user name )
   
   As result you can either see exception or data populated into users table that was created at the beginning of the program


## Config File format
 Column layout is very simple  - text file with lines specifying column name 
 Line number is position of column in csv file
 name of the file corresponds to user name - assumption is that they are unique for the purpose of this exersize
 
 ## Assumptions
 There are 8 columns of interest taken from original document
If column value not provided by the user ( doesn't exists in config) - it will be populated with empty values
All defined as strings (including date) and not tested for valid value ( such as valid date or UUID)
 

