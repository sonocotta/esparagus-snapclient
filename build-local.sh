#!/bin/bash

# Local build script that mimics the GitHub Actions workflow
# Builds all configurations and updates artifacts locally

set -e  # Exit on error

echo "=========================================="
echo "Local Build Script for Snapcast Binaries"
echo "=========================================="
echo ""

# Get the script directory (project root)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if docker is running
if ! docker info > /dev/null 2>&1; then
    echo "Error: Docker is not running. Please start Docker and try again."
    exit 1
fi

# Generate date string
datestring=$(date '+%Y-%m-%d')
echo "Build date: $datestring"
echo ""

# Create binaries directory if it doesn't exist
mkdir -p docs/artifacts/binaries

# Run the build in Docker container
echo "Starting Docker container with ESP-IDF v5.1.1..."
echo ""

docker run --rm \
    -v "$SCRIPT_DIR/snapclient:/project" \
    -v "$SCRIPT_DIR/configs:/project/configs" \
    -v "$SCRIPT_DIR/docs/artifacts:/project/artifacts" \
    -w /project \
    espressif/idf:v5.1.1 \
    /bin/bash -c "
        set -e
        . /opt/esp/idf/export.sh
        
        echo 'Building all configurations...'
        echo ''
        
        for config_file in configs/sdkconfig.*; do
            config=\${config_file#configs/sdkconfig.}
            
            # Skip old configs
            if [[ \"\$config\" == \"old\" ]]; then
                continue
            fi
            
            echo '=========================================='
            echo \"Building: \$config\"
            echo '=========================================='
            
            # Use separate build directory for each config
            build_dir=\"build-\$config\"
            
            # Copy config
            cp \$config_file sdkconfig
            
            # Build with custom build directory
            idf.py -B \$build_dir build
            
            # Copy binaries to artifacts
            cp \$build_dir/snapclient.bin artifacts/binaries/\$config-snapclient-$datestring-snapclient.bin
            cp \$build_dir/storage.bin artifacts/binaries/\$config-snapclient-$datestring-storage.bin
            cp \$build_dir/ota_data_initial.bin artifacts/binaries/\$config-snapclient-$datestring-ota_data_initial.bin
            cp \$build_dir/bootloader/bootloader.bin artifacts/binaries/\$config-snapclient-$datestring-bootloader.bin
            cp \$build_dir/partition_table/partition-table.bin artifacts/binaries/\$config-snapclient-$datestring-partition-table.bin
            
            echo \"✓ Build completed for \$config\"
            echo ''
        done
        
        echo '=========================================='
        echo 'All builds completed successfully!'
        echo '=========================================='
    "

# Update manifest files
echo ""
echo "Updating manifest files..."
for template in docs/artifacts/templates/manifest-*-template.json; do
    if [ -f "$template" ]; then
        config=$(basename "$template" | sed 's/manifest-//' | sed 's/-template.json//')
        sed -e "s/latest/$datestring/g" "$template" > "docs/artifacts/manifest-$config-latest.json"
        echo "✓ Updated manifest-$config-latest.json"
    fi
done

echo ""
echo "=========================================="
echo "Build completed successfully!"
echo "=========================================="
echo ""
echo "Binaries are in: docs/artifacts/binaries/"
echo "Manifests updated with date: $datestring"
echo ""
echo "To commit these changes:"
echo "  git add docs/artifacts"
echo "  git commit -m 'Update binaries - $datestring'"
echo "  git push"
echo ""
