<?php
/**
 * UberCoova settings form.
 */
function ubercoova_settings() {
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

  return system_settings_form($form);
}
