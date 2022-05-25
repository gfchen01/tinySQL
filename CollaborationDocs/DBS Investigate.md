# DBS Investigate

1. A method to store the position of a record is to simply store its block_id in the file
2. Bp tree shouldn't care about the pointer type.
3. Remembering the page id and make the record manager to find it is an efficient strategy. 