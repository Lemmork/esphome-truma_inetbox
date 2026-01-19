from typing import Any, Callable, Dict, List, Optional, Union

import esphome.final_validate as fv
from esphome import core
from esphome.const import CONF_ID

def count_id_usage(
    property_to_update: Union[str, List[str]],
    property_to_count: Union[str, List[str]],
    property_value: type,
) -> Callable[[Dict[str, Any]], Dict[str, Any]]:
    """Validator that counts a configuration property from another entity, for use with FINAL_VALIDATE_SCHEMA.
    If a property is already set, it will not be updated.

    This helper automatically inherits a count-based property from other entities in the configuration.
    For example, it can automatically set the number of heater units based on how many heater
    entities are configured elsewhere.

    Args:
        property_to_update: Path or list of path segments to the property to update
        property_to_count: Name or list of names of properties to count in the full config
        property_value: The type/value to match when counting properties

    Returns:
        A validator function that can be used with FINAL_VALIDATE_SCHEMA
    """

    def _walk_config(config: Dict[str, Any], path: Union[str, List[str]]) -> Any:
        """Navigate through nested config dict/list using a path"""
        walk = [path] if not isinstance(path, list) else path
        for item_or_index in walk:
            config = config[item_or_index]
        return config

    def _count_config_value(
        config: Any,
        conf_name_list: List[str],
        conf_value: type,
    ) -> int:
        """Recursively count occurrences of specific config values in nested structures"""
        ret = 0

        if isinstance(config, (list, tuple)):
            for config_item in config:
                ret += _count_config_value(config_item, conf_name_list, conf_value)
            for conf_name in conf_name_list:
                if (
                    conf_name in config
                    and isinstance(config, tuple)
                    and config[0] == conf_name
                    and isinstance(config[1], core.ID)
                    and config[1].type is conf_value
                ):
                    ret += 1
        elif isinstance(config, dict):
            for config_item in config.items():
                ret += _count_config_value(config_item, conf_name_list, conf_value)

        return ret

    def inherit_property(config: Dict[str, Any]) -> Dict[str, Any]:
        """Main validator function that performs the property inheritance"""
        # Ensure `property_to_count` is a list
        property_to_count_list: List[str] = (
            [property_to_count]
            if not isinstance(property_to_count, list)
            else property_to_count
        )

        # Split the property into its path and name
        if not isinstance(property_to_update, list):
            property_path: List[str] = []
            property_name: str = property_to_update
        else:
            property_path = property_to_update[:-1]
            property_name = property_to_update[-1]

        # Check if the property is accessible
        try:
            config_part = _walk_config(config, property_path)
        except KeyError:
            return config

        # Only update the property if it does not exist yet
        if property not in config_part:
            fconf = fv.full_config.get()

            # Count matching properties in the full configuration
            count = _count_config_value(
                fconf, property_to_count_list, property_value)

            path = fconf.get_path_for_id(config[CONF_ID])[:-1]
            this_config = _walk_config(
                fconf.get_config_for_path(path), property_path
            )
            this_config[property] = count
        return config

    return inherit_property
