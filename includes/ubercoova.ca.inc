<?php
/**
 * Implementation of hook_ca_predicate().
 */
function ubercoova_ca_predicate() {
  $predicates = array();

  $predicates['ubercoova_update_quota'] = array(
    '#title' => t('Update user quota'),
    '#description' => t('Transfer purchased quota to FreeRADIUS.'),
    '#class' => 'quota',
    '#trigger' => 'uc_order_status_update',
    '#status' => 1,
    '#conditions' => array(
      '#operator' => 'AND',
      '#conditions' => array(
        array(
          '#name' => 'uc_order_status_condition',
          '#title' => t('If the original order status was not Completed.'),
          '#argument_map' => array(
            'order' => 'order',
          ),
          '#settings' => array(
            'negate' => TRUE,
            'order_status' => 'completed',
          ),
        ),
        array(
          '#name' => 'uc_order_status_condition',
          '#title' => t('If the updated order status is Completed.'),
          '#argument_map' => array(
            'order' => 'updated_order',
          ),
          '#settings' => array(
            'order_status' => 'completed',
          ),
        ),
        array(
          '#name' => 'ubercoova_condition_check_quota',
          '#title' => t('If the order contains products that provide quota.'),
          '#argument_map' => array(
            'order' => 'updated_order',
          ),
          '#settings' => array(),
        ),
      ),
    ),
    '#actions' => array(
      array(
        '#name' => 'ubercoova_action_update_quota',
        '#title' => t('Transfer purchased quota to radius account.'),
        '#argument_map' => array(
          'order' => 'updated_order',
        ),
      ),
    ),
  );
  return $predicates;
}


/**
 * Implementation of hook_ca_condition().
 *
 * This helps set up a CA to add quota to a user's account without
 * a hardcoded uc_checkout hook.
 *
 * The order contains a product with download quota.
 */
function ubercoova_ca_condition() {
  $conditions = array();
  
  $conditions['ubercoova_condition_check_quota'] = array(
    '#title' => t('The cart contains products that affect the user\'s download quota'),
    '#category' => t('Order'),
    '#callback' => 'ubercoova_condition_check_quota',
    '#arguments' => array(
      'order' => array(
        '#entity' => 'uc_order',
        '#title' => t('Order'),
      ),
    ),
  );

  return $conditions;
}

/**
 * Implementation of hook_ca_action().
 *
 * This helps set up a CA to add quota to a user's account without
 * a hardcoded uc_checkout hook.
 *
 * Provide a callback to add purchased quota to a user's radius account.
 */
function ubercoova_ca_action() {
  $actions = array();
  
  $actions['ubercoova_action_update_quota'] = array(
    '#title' => t('Transfer purchased quota to radius account'),
    '#category' => t('Checkout'),
    '#callback' => 'ubercoova_action_update_quota',
    '#arguments' => array(
      'order' => array(
        '#entity' => 'uc_order',
        '#title' => t('Order'),
      ),
    ),
  );

  return $actions;
}

/**
 * Returns true if the order has a product that contains quota.
 *
 * This helps set up a CA to add quota to a user's account without
 * a hardcoded uc_checkout hook.
 * 
 * @return boolean
 *   TRUE or FALSE
 */
function ubercoova_condition_check_quota($order) {
  $result = FALSE;
  foreach ($order->products as $product) {
    $node = node_load($product->nid);
    if (!empty($node->field_ubercoova_quota[0]['value'])) {
      $result = TRUE;
      break;
    }
  }
  return $result;
}

/*
 * A callback to add purchased quota to a user account.
 *
 * @param $order
 *   A valid ubercart order object.
 */
function ubercoova_action_update_quota($order) {

  $account = user_load($order->uid);

  // Start with 0 quota.
  $purchased = (float)0;

  // For each product in this order, add the purchased quota to the running total.
  foreach ($order->products as $product) {
    $node = node_load($product->nid);
    if (!empty($node->field_ubercoova_quota[0]['value'])) {
      $purchased += ($product->qty * $node->field_ubercoova_quota[0]['value']);
    }
  }

  // We have a purchased quota total in GB now.
  // Convert to bytes before adding to radius.
  $purchased *= pow(UBERCOOVA_KILOBYTE, 3);

  // Load vars from the default database.
  $table = variable_get('ubercoova_table', '');
  $attribute = variable_get('ubercoova_attribute_quota', '');

  db_set_active('radius');
  // Need to use floatval() because on 32bit int is limited in range from
  // negative to positive 2GB. Oops.
  $remaining = floatval(db_result(db_query("SELECT value FROM %s WHERE username = '%s' AND attribute = '%s' AND op = ':='", $table, $account->name, $attribute)));
  $quota = $remaining + $purchased;

  db_query("UPDATE %s SET value = %f WHERE username = '%s' AND attribute = '%s' AND op = ':='", $table, $quota, $account->name, $attribute);
  db_set_active();

  // Watchdog log it, so if there is fail we can check.
  watchdog('ubercoova', 'Had !remaining. Bought !purchased. Now has !quota quota.', array('!remaining' => format_size($remaining), '!purchased' => format_size($purchased), '!quota' => format_size($quota)), WATCHDOG_INFO, NULL);
}
