<<COMMENT
What the Script Should Do:
1. Call build.sh to compile the main program and all plugins.
2. Run several test cases by providing input (via echo or a here-document) and checking
the expected output.
3. Check for both positive cases (correct inputs that should be processed) and
negative cases (incorrect usage, missing plugins, broken behavior).
4. Print clear success/failure messages for each test.
5. Exit with a non-zero code if any test fails. 
COMMENT