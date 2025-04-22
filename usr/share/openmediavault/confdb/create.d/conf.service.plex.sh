#!/bin/bash
set -e

. /usr/share/openmediavault/scripts/helper-functions

SERVICE_XPATH_NAME="plex"
SERVICE_XPATH="/config/services/${SERVICE_XPATH_NAME}"

if ! omv_config_exists "${SERVICE_XPATH}"; then
    echo "Creating default Plex configuration..."
    
    # Add base configuration node
    omv_config_add_node "/config/services" "${SERVICE_XPATH_NAME}"
    
    # General settings
    omv_config_add_key "${SERVICE_XPATH}" "enable" "0"
    omv_config_add_key "${SERVICE_XPATH}" "installation_type" "docker"
    omv_config_add_key "${SERVICE_XPATH}" "port" "32400"
    omv_config_add_key "${SERVICE_XPATH}" "remote_access" "1"
    omv_config_add_key "${SERVICE_XPATH}" "auto_update" "0"
    
    # Docker specific settings
    omv_config_add_key "${SERVICE_XPATH}" "docker_image" "plexinc/pms-docker"
    omv_config_add_key "${SERVICE_XPATH}" "docker_tag" "latest"
    omv_config_add_key "${SERVICE_XPATH}" "docker_network" "host"
    
    # Library configuration
    omv_config_add_key "${SERVICE_XPATH}" "claim_token" ""
    
    # Advanced settings
    omv_config_add_key "${SERVICE_XPATH}" "transcoder_path" "/tmp"
    omv_config_add_key "${SERVICE_XPATH}" "verbose_logging" "0"
    
    echo "Default Plex configuration created successfully."
fi

exit 0
