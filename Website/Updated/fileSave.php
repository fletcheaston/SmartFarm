<!DOCTYPE html>
<html>
<body>

<?php
session_start();
$_SESSION["my_data"] = $_POST['program'];
echo "data recieved = " . $_POST['program'];
?>

</body>
</html>
