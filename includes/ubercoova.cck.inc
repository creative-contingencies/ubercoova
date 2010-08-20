<?php
/**
 * Define CCK quota field for use in hook_install()
 */
function _ubercoova_export()
  return
    array (
      'label' => 'Quota',
      'field_name' => 'field_ubercoova_quota',
      'type' => 'number_integer',
      'widget_type' => 'number',
      'change' => 'Change basic information',
      'weight' => '31',
      'description' => 'Enter the quota (in megabytes) that should be assigned to the user when the buy this product.',
      'default_value' => 
      array (
        0 => 
        array (
          'value' => '',
          '_error_element' => 'default_value_widget][field_ubercoova_quota][0][value',
        ),
      ),
      'default_value_php' => '',
      'default_value_widget' => NULL,
      'required' => 1,
      'multiple' => '0',
      'min' => '0',
      'max' => '',
      'prefix' => '',
      'suffix' => 'MB',
      'allowed_values' => '',
      'op' => 'Save field settings',
      'module' => 'number',
      'widget_module' => 'number',
      'columns' => 
      array (
        'value' => 
        array (
          'type' => 'int',
          'not null' => false,
          'sortable' => true,
        ),
      ),
      'display_settings' => 
      array (
        'weight' => '31',
        'parent' => '',
        'label' => 
        array (
          'format' => 'hidden',
        ),
        'teaser' => 
        array (
          'format' => 'hidden',
          'exclude' => 0,
        ),
        'full' => 
        array (
          'format' => 'hidden',
          'exclude' => 0,
        ),
        4 => 
        array (
          'format' => 'default',
          'exclude' => 0,
        ),
        'token' => 
        array (
          'format' => 'default',
          'exclude' => 0,
        ),
      ),
    );
}
