<?php
$dir = "/var/log";
if ( !file_exists($dir) ) {
    mkdir ($dir, 0744);
}

session_start();

file_put_contents ($dir.'/program.txt', $_POST['program']);

$_SESSION["my_data"] = $_POST['program'];
echo "data recieved = " . $_POST['program'];
?>
