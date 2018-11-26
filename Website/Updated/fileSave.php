<!DOCTYPE html>
<html>
<body>

<?php
session_start();
$_SESSION["my_data"] = $_POST['action'];
echo "data recieved = " . $_POST['action'];
?>

</body>
</html>
