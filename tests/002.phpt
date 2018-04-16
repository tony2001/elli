--TEST--
elli_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('elli')) {
	echo 'skip';
}
?>
--FILE--
<?php 
$ret = elli_test1();

var_dump($ret);
?>
--EXPECT--
The extension elli is loaded and working!
NULL
