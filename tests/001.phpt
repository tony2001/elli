--TEST--
Check if elli is loaded
--SKIPIF--
<?php
if (!extension_loaded('elli')) {
	echo 'skip';
}
?>
--FILE--
<?php 
echo 'The extension "elli" is available';
?>
--EXPECT--
The extension "elli" is available
