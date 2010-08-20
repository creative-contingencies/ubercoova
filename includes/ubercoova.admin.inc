<?php
/**
 * UberCoova settings form.
 */
function ubercoova_settings() {

  // This whole thing will fail if there is no 'radius' $db_url in settings.php. So check for that first.
  global $db_url;
  if (empty($db_url['radius'])) {
    drupal_set_message(t('No $db_url named radius was found in your settings.php file. Please add one.'), 'error');
    return;
  }

  module_load_include('inc', 'ubercoova', 'help/ubercoova.admin.help');

  $form = array();
  $form['help'] = array(
    '#type' => 'fieldset',
    '#title' => 'Help',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['help']['ubercoova_help'] = array(
    '#type'  => 'markup',
    '#value' => ubercoova_settings_help(),
  );

  // Switch to the radius database and grab some options.
  db_set_active('radius');

  // Create an option list of tables that we might want to use.
  $options = array('' => t('<none>'));
  $result = db_query("SHOW TABLES");
  while ($row = db_result($result)) {
    $options[$row] = $row;
  }

  // And switch back to the normal database, so we can fetch variables.
  db_set_active();
  $table = variable_get('ubercoova_table', '');

  $form['ubercoova_table'] = array(
    '#type' => 'select',
    '#title' => 'Table',
    '#description' => t('Select the table that holds radius account and quota information for each user.'),
    '#options' => $options,
    '#default_value' => $table,
  );

  // Make sure the user has chosen a table before the other fields are shown.
  if (empty($table)) {
    return system_settings_form($form);
  }

  // Switch to the radius database and grab the available attributes.
  db_set_active('radius');

  // Field definition details for FreeRADIUS.
  $options = array('' => t('<none>'));
  $result = db_query("SELECT DISTINCT(attribute) FROM " . $table . " ORDER BY attribute ASC");
  while ($row = db_fetch_array($result)) {
    $options[$row['attribute']] = $row['attribute'];
  }

  // And switch back to the normal database.
  db_set_active();

  $form['ubercoova_attribute_password'] = array(
    '#type' => 'select',
    '#title' => 'Password Attribute',
    '#description' => t('Select the %table attribute that contains the user\'s password.', array('%table' => $table)),
    '#options' => $options,
    '#default_value' => variable_get('ubercoova_attribute_password', ''),
  );
  $form['ubercoova_attribute_quota'] = array(
    '#type' => 'select',
    '#title' => 'Quota Attribute',
    '#description' => t('Select the %table attribute that contains the user\'s quota.', array('%table' => $table)),
    '#options' => $options,
    '#default_value' => variable_get('ubercoova_attribute_quota', ''),
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
}
