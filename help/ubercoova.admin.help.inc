<?php
/**
 * Helpfile for ubercoova.
 */

/**
 * Creates the help text for the Ubercoova settings page
 *
 * @return
 *   HTML with the help text
 */
function ubercoova_settings_help() {
  return t('
<p>To make use of the <em>ubercoova</em> module, Drupal needs to be able to access the FreeRADIUS backend database.</p>
<p>You can configure this database by editing your settings.php file.</p>
<p>Modify the $db_url variable, so that instead of a string it is an array. The array should have at least a \'default\' key that contains the existing url for the Drupal database.
First, change:
<pre>
  $db_url = \'mysqli://username:password@localhost/databasename\';
</pre>
<p>To:</p>
<pre>
  $db_url = array();
  $db_url[\'default\'] = \'mysqli://username:password@localhost/drupal\';
</pre>
<p>You can now append the database url for the freeradius database below the default url. You should give it the key \'radius\'.
<pre>
  $db_url = array();
  $db_url[\'default\'] = \'mysqli://username:password@localhost/drupal\';
  $db_url[\'radius\'] = \'mysqli://raduser:secret@localhost/radius\';
</pre>
<p>The <em>raduser</em> account needs to be able to read from and write to the table with account credentials and attributes.</p>
  ');
}
