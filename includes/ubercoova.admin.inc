<?php
/**
 * UberCoova settings form.
 */
function ubercoova_settings() {

  // This whole thing will fail if there is no 'radius' $db_url in settings.php.
  // So check for that first.
  global $db_url;
  if (!is_array($db_url) || empty($db_url['radius'])) {
    drupal_set_message(t('This module requires additional configuration. No $db_url named radius was found in your settings.php file. Please add one.'), 'error');
    $display_help = TRUE;
  }

  module_load_include('inc', 'ubercoova', 'help/ubercoova.admin.help');

  // If the radius db_url wasn't found, expand this help section.
  $form = array();
  $form['help'] = array(
    '#type' => 'fieldset',
    '#title' => 'Help',
    '#collapsible' => TRUE,
    '#collapsed' => ($display_help) ? FALSE : TRUE,
  );
  $form['help']['ubercoova_help'] = array(
    '#type'  => 'markup',
    '#value' => ubercoova_settings_help(),
  );

  // If the radius db_url wasn't found, display only the help section.
  if ($display_help) {
    return system_settings_form($form);
  }

  // Default table, saves us a db_switch later.
  $table = variable_get('ubercoova_table', '');
  $table_acct = variable_get('ubercoova_table_accounting', '');
  $table_ug = variable_get('ubercoova_table_usergroup', '');

  // Switch to the radius database and check that the table exists. We re-do
  // this in case $db_url has changed,
  db_set_active('radius');
  $table = db_table_exists($table) ? $table : '';

  // Create an option list of tables that we might want to use.
  $tables = array('' => t('<none>'));
  $result = db_query("SHOW TABLES");
  while ($row = db_result($result)) {
    $tables[$row] = $row;
  }

  $form['ubercoova_table'] = array(
    '#type' => 'select',
    '#title' => 'RADIUS Check Table',
    '#description' => t('Select the table that holds radius account and quota information for each user.'),
    '#options' => $tables,
    '#default_value' => $table,
  );

  // Make sure the user has chosen a table before the other fields are
  // shown. If not, switch back to default db and exit.
  if (empty($table)) {
    db_set_active();
    return system_settings_form($form);
  }

  // Field definition details of check table for FreeRADIUS.
  $attributes = array('' => t('<none>'));
  $result = db_query("SELECT DISTINCT(attribute) FROM %s ORDER BY attribute ASC", $table);
  while ($row = db_fetch_array($result)) {
    $attributes[$row['attribute']] = $row['attribute'];
  }

  // And switch back to the normal database.
  db_set_active();

  $form['ubercoova_attribute_password'] = array(
    '#type' => 'select',
    '#title' => 'Password Attribute',
    '#description' => t('Select the %table attribute that contains the user\'s password. This allows users to login to Drupal using their radius credentials.', array('%table' => $table)),
    '#options' => $attributes,
    '#default_value' => variable_get('ubercoova_attribute_password', ''),
  );
  $form['ubercoova_attribute_quota'] = array(
    '#type' => 'select',
    '#title' => 'Quota Attribute',
    '#description' => t('Select the %table attribute that contains the user\'s quota. This allows users to purchase download quota.', array('%table' => $table)),
    '#options' => $attributes,
    '#default_value' => variable_get('ubercoova_attribute_quota', ''),
  );

  // Switch to the radius database and check that the table exists. Maybe this
  // could be done above and the switch back to the default removed? Not
  // familiar enough w/ drupal to know if this is courting disaster. LOOKS safe
  // enough
  db_set_active('radius');
  $table_acct = db_table_exists($table_acct) ? $table_acct : '';
  $table_ug = db_table_exists($table_ug) ? $table_ug : '';
  
  // We need to be able to get accounting info from radius to properly display
  // users' remaining quota on their account page.
  $form['ubercoova_table_accounting'] = array(
    '#type' => 'select',
    '#title' => 'RADIUS Accounting Table',
    '#description' => t('Select the table that holds RADIUS session accounting information for each user.'),
    '#options' => $tables,
    '#default_value' => $table_acct,
  );

  // Make sure the user has chosen a table before the next field is
  // shown. If not, switch back to default db and exit.
  if (empty($table)) {
    db_set_active();
    return system_settings_form($form);
  }

  // Field definition details for accounting table of FreeRADIUS.
  $fields_acct = array('' => t('<none>'));
  $result = db_query("SHOW COLUMNS FROM %s LIKE '%octets'", $table_acct);
  while ($row = db_fetch_array($result)) {
    $fields_acct[$row['Field']] = $row['Field'];
  }

  // And switch back to the normal database.
  db_set_active();

  $form['ubercoova_field_accounting'] = array(
    '#type' => 'select',
    '#title' => 'RADIUS Accounting Table field',
    '#description' => t('Select the %table_acct column that contains the downstream traffic (clients\' perspective) accounting data. This option is provided to adjust for the CoovaChilli swapoctets command.', array('%table_acct' => $table_acct)),
    '#options' => $fields_acct,
    '#default_value' => variable_get('ubercoova_field_accounting', ''),
  );

  $form['ubercoova_table_usergroup'] = array(
    '#type' => 'select',
    '#title' => 'RADIUS UserGroup Table',
    '#description' => t('Select the table that holds RADIUS group information for users.'),
    '#options' => $tables,
    '#default_value' => $table_ug,
  );

  $form['ubercoova_radclient'] = array(
    '#type' => 'textfield',
    '#title' => 'RADIUS Client Binary',
    '#description' => t('Enter the full path to the <em>radclient</em> binary. This can be used to disconnect hotspot users via the website.'),
    '#default_value' => variable_get('ubercoova_radclient', ''),
  );

  $roles = array(0 => '<none>');
  $roles += user_roles(TRUE);
  $form['ubercoova_rid'] = array(
    '#type' => 'select',
    '#title' => 'Assign Role',
    '#description' => t('Assign this role to a Drupal user when it is newly created from the radius database.'),
    '#options' => $roles,
    '#default_value' => variable_get('ubercoova_rid', 0),
  );

  $form['topup'] = array(
    '#type' => 'fieldset',
    '#title' => 'Top up Settings',
    '#collapsible' => TRUE,
    '#description' => t('These settings affect the bandwidth top-up menu links.'),
  );

  $form['topup']['ubercoova_price_gb'] = array(
    '#type' => 'textfield',
    '#title' => 'Price',
    '#size' => 6,
    '#description' => t('Price per GiB of quota. This is used when topping up an amount of money via the administration screen, not to calculate prices in the shop.'),
    '#default_value' => variable_get('ubercoova_price_gb', ''),
    '#field_prefix' => t('$'),
    '#field_suffix' => t('per GiB'),
  );

  // Links for the topup menu item.
  $form['topup']['ubercoova_topup_anonymous'] = array(
    '#type' => 'textfield',
    '#title' => 'Anonymous User',
    '#description' => t('Enter a menu title for the bandwidth top-up link displayed to anonymous users. These users will be required to login prior to seeing the list of products.'),
    '#default_value' => variable_get('ubercoova_topup_anonymous', 'Login and top up my bandwidth'),
  );
  $form['topup']['ubercoova_topup_authenticated'] = array(
    '#type' => 'textfield',
    '#title' => 'Authenticated User',
    '#description' => t('Enter a menu title for the bandwidth top-up link displayed to authenticated users.'),
    '#default_value' => variable_get('ubercoova_topup_anonymous', 'Top up my bandwidth'),
  );


  $form['email'] = array(
    '#type' => 'fieldset',
    '#title' => 'Email Settings',
    '#collapsible' => TRUE,
    '#description' => t('These settings affect whether users are assigned email addresses when they are created in Drupal from RADIUS data.'),
  );
  $form['email']['ubercoova_email_uname'] = array(
    '#type' => 'textfield',
    '#title' => 'Email Username',
    '#description' => t('All users imported from RADIUS will be given this as their email username. You can leave this blank if you want to use the username imported from RADIUS.'),
    '#default_value' => variable_get('ubercoova_email_uname', ''),
  );
  $form['email']['ubercoova_email_domain'] = array(
    '#type' => 'textfield',
    '#title' => 'Email Domain',
    '#description' => t('Append this email domain to the username to generate email addresses for users. You can leave this blank if you don\'t want to generate email addresses.'),
    '#default_value' => variable_get('ubercoova_email_domain', ''),
  );
  $form['email']['ubercoova_email_modify'] = array(
    '#type' => 'radios',
    '#title' => 'Edit Emails',
    '#description' => t('Allow users to modify their own generated email addresses? Any changes a user makes will also be saved to the %table table.', array('%table' => $table)),
    '#options' => array(0 => 'No', 1 => 'Yes'),
    '#default_value' => variable_get('ubercoova_email_modify', 0),
  );

  $form['cleanup'] = array(
    '#type' => 'fieldset',
    '#title' => 'Cleanup Settings',
    '#collapsible' => TRUE,
    '#description' => t('These settings affect whether users are removed from RADIUS when they are deleted from Drupal.'),
  );
  $form['cleanup']['ubercoova_cleanup'] = array(
    '#type' => 'radios',
    '#title' => 'Cleanup',
    '#description' => t('Delete users from RADIUS when they are deleted from Drupal?'),
    '#options' => array(
      0 => t('Leave user details in the RADIUS database.'),
      1 => t('Delete all users details from the RADIUS database.'),
    ),
    '#default_value' => variable_get('ubercoova_cleanup', 0),
  );
  array_shift($tables);
  $form['cleanup']['ubercoova_cleanup_tables'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Cleanup Tables',
    '#description' => t('Select all RADIUS tables that require cleanup when a user is deleted from Drupal. This will delete all their entries from the selected tables.'),
    '#options' => $tables,
    '#default_value' => variable_get('ubercoova_cleanup_tables', array()),
  );
  
  return system_settings_form($form);
}

function ubercoova_settings_validate(&$form, &$form_state) {
  // Check if we have a table and - if so - whether that table contains an attribute column.
  if (!empty($form_state['values']['ubercoova_table'])) {
    db_set_active('radius');
    $attributes = db_column_exists($form_state['values']['ubercoova_table'], 'attribute');
    db_set_active();

    if (empty($attributes)) {
      form_set_error('ubercoova_table', t('The table %table does not contain an attribute column', array('%table' => $form_state['values']['ubercoova_table'])));
    }
  }

  if (!empty($form_state['values']['ubercoova_email_domain']) && (strpos($form_state['values']['ubercoova_email_domain'], '@') !== 0)) {
    form_set_error('email][ubercoova_email_domain', t('An email domain must start with an @ symbol.'));
  }

  if (!empty($form_state['values']['ubercoova_radclient']) && !file_exists($form_state['values']['ubercoova_radclient'])) {
    form_set_error('ubercoova_radclient', t('The radclient binary does not exist in this location'));
  }

  if (empty($form_state['values']['ubercoova_topup_anonymous'])) {
    form_set_error('ubercoova_topup_anonymous', t('The anonymous menu link must have a title.'));
  }

  if (empty($form_state['values']['ubercoova_topup_authenticated'])) {
    form_set_error('ubercoova_topup_anonymous', t('The authenticated menu link must have a title.'));
  }

  if (empty($form_state['values']['ubercoova_price_gb'])) {
    form_set_error('ubercoova_price_gb', t('Please enter a price per GB'));
  }
  else if (!floatval($form_state['values']['ubercoova_price_gb'])) {
    form_set_error('ubercoova_price_gb', t('Please enter a valid price per GB'));
  }
}
