<?php
$dir = "../Platformio/src";
if ( !file_exists($dir) ) {
    mkdir ($dir, 0744);
}

session_start();

file_put_contents ($dir.'/program.ino', $_POST['program']);

echo "Program saved!";

?>
