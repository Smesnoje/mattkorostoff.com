<?php

/**
 * @file
 * Provides views data for taxonomy.module.
 */

use Drupal\field\FieldStorageConfigInterface;

/**
 * Implements hook_views_data_alter().
 */
function taxonomy_views_data_alter(&$data) {
  $data['node']['term_node_tid'] = array(
    'title' => t('Taxonomy terms on node'),
    'help' => t('Relate nodes to taxonomy terms, specifying which vocabulary or vocabularies to use. This relationship will cause duplicated records if there are multiple terms.'),
    'relationship' => array(
      'id' => 'node_term_data',
      'label' => t('term'),
      'base' => 'taxonomy_term_data',
    ),
    'field' => array(
      'title' => t('All taxonomy terms'),
      'help' => t('Display all taxonomy terms associated with a node from specified vocabularies.'),
      'id' => 'taxonomy_index_tid',
      'no group by' => TRUE,
      'click sortable' => FALSE,
    ),
  );

  $data['node']['term_node_tid_depth'] = array(
    'help' => t('Display content if it has the selected taxonomy terms, or children of the selected terms. Due to additional complexity, this has fewer options than the versions without depth.'),
    'real field' => 'nid',
    'argument' => array(
      'title' => t('Has taxonomy term ID (with depth)'),
      'id' => 'taxonomy_index_tid_depth',
      'accept depth modifier' => TRUE,
    ),
    'filter' => array(
      'title' => t('Has taxonomy terms (with depth)'),
      'id' => 'taxonomy_index_tid_depth',
    ),
  );

  $data['node']['term_node_tid_depth_modifier'] = array(
    'title' => t('Has taxonomy term ID depth modifier'),
    'help' => t('Allows the "depth" for Taxonomy: Term ID (with depth) to be modified via an additional contextual filter value.'),
    'argument' => array(
      'id' => 'taxonomy_index_tid_depth_modifier',
    ),
  );
}

/**
 * Implements hook_field_views_data().
 *
 * Views integration for taxonomy_term_reference fields. Adds a term relationship to the default
 * field data.
 *
 * @see field_views_field_default_views_data()
 */
function taxonomy_field_views_data(FieldStorageConfigInterface $field_storage) {
  $data = field_views_field_default_views_data($field_storage);
  foreach ($data as $table_name => $table_data) {
    foreach ($table_data as $field_name => $field_data) {
      if (isset($field_data['filter']) && $field_name != 'delta') {
        $data[$table_name][$field_name]['filter']['id'] = 'taxonomy_index_tid';
        $allowed_values = $field_storage->getSetting('allowed_values');
        $data[$table_name][$field_name]['filter']['vocabulary'] = $allowed_values[0]['vocabulary'];
      }
    }

    // Add the relationship only on the tid field.
    $field_name = $field_storage->getName();
    $data[$table_name][$field_name . '_target_id']['relationship'] = array(
      'id' => 'standard',
      'base' => 'taxonomy_term_data',
      'base field' => 'tid',
      'label' => t('term from !field_name', array('!field_name' => $field_name)),
    );

  }

  return $data;
}

/**
 * Implements hook_field_views_data_views_data_alter().
 *
 * Views integration to provide reverse relationships on term references.
 */
function taxonomy_field_views_data_views_data_alter(array &$data, FieldStorageConfigInterface $field_storage) {
  $field_name = $field_storage->getName();
  $entity_type_id = $field_storage->getTargetEntityTypeId();
  $entity_manager = \Drupal::entityManager();
  $entity_type = $entity_manager->getDefinition($entity_type_id);
  $pseudo_field_name = 'reverse_' . $field_name . '_' . $entity_type_id;
  /** @var \Drupal\Core\Entity\Sql\DefaultTableMapping $table_mapping */
  $table_mapping = $entity_manager->getStorage($entity_type_id)->getTableMapping();

  list($label) = field_views_field_label($entity_type_id, $field_name);

  $data['taxonomy_term_data'][$pseudo_field_name]['relationship'] = array(
    'title' => t('@entity using @field', array('@entity' => $entity_type->getLabel(), '@field' => $label)),
    'help' => t('Relate each @entity with a @field set to the term.', array('@entity' => $entity_type->getLabel(), '@field' => $label)),
    'id' => 'entity_reverse',
    'field_name' => $field_name,
    'entity_type' => $entity_type_id,
    'field table' => $table_mapping->getDedicatedDataTableName($field_storage),
    'field field' => $field_name . '_target_id',
    'base' => $entity_type->getBaseTable(),
    'base field' => $entity_type->getKey('id'),
    'label' => t('!field_name', array('!field_name' => $field_name)),
    'join_extra' => array(
      0 => array(
        'field' => 'deleted',
        'value' => 0,
        'numeric' => TRUE,
      ),
    ),
  );
}

/**
 * Implements hook_views_plugins_argument_validator_alter().
 *
 * Extend the generic entity argument validator.
 *
 * @see \Drupal\views\Plugin\views\argument_validator\Entity
 */
function taxonomy_views_plugins_argument_validator_alter(array &$plugins) {
  $plugins['entity:taxonomy_term']['title'] = t('Taxonomy term ID');
  $plugins['entity:taxonomy_term']['class'] = 'Drupal\taxonomy\Plugin\views\argument_validator\Term';
  $plugins['entity:taxonomy_term']['provider'] = 'taxonomy';
}
