#!/bin/bash
# Log management script for ESP32 monitor output
# Usage: ./manage_logs.sh [command] [options]

set -e

# Configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOGS_DIR="$PROJECT_DIR/logs"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_color() {
    local color=$1
    shift
    echo -e "${color}$*${NC}"
}

# Function to show usage
show_usage() {
    echo "ESP32 Log Management Script"
    echo ""
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  list                  - List all log files"
    echo "  latest               - Show the most recent log file"
    echo "  view [filename]      - View a specific log file"
    echo "  tail [filename]      - Follow a log file in real-time"
    echo "  search [pattern]     - Search for pattern in all logs"
    echo "  clean [days]         - Remove logs older than specified days (default: 7)"
    echo "  stats                - Show log statistics"
    echo "  help                 - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 list"
    echo "  $0 view gpio_Debug_20241220_143022.log"
    echo "  $0 search \"ERROR\""
    echo "  $0 clean 3"
    echo "  $0 tail latest"
}

# Function to list log files
list_logs() {
    print_color $BLUE "=== ESP32 Monitor Log Files ==="
    
    if [ ! -d "$LOGS_DIR" ]; then
        print_color $YELLOW "No logs directory found at: $LOGS_DIR"
        return 1
    fi
    
    local logs=($(find "$LOGS_DIR" -name "*.log" -type f | sort -r))
    
    if [ ${#logs[@]} -eq 0 ]; then
        print_color $YELLOW "No log files found in: $LOGS_DIR"
        return 1
    fi
    
    echo ""
    printf "%-40s %-20s %-10s %s\n" "Filename" "Modified" "Size" "Example Type"
    printf "%-40s %-20s %-10s %s\n" "--------" "--------" "----" "------------"
    
    for log in "${logs[@]}"; do
        local filename=$(basename "$log")
        local modified=$(stat -c "%Y" "$log" 2>/dev/null || stat -f "%m" "$log" 2>/dev/null || echo "0")
        local modified_date=$(date -d "@$modified" "+%Y-%m-%d %H:%M" 2>/dev/null || date -r "$modified" "+%Y-%m-%d %H:%M" 2>/dev/null || echo "Unknown")
        local size=$(du -h "$log" | cut -f1)
        local example_type=$(echo "$filename" | cut -d'_' -f1)
        
        printf "%-40s %-20s %-10s %s\n" "$filename" "$modified_date" "$size" "$example_type"
    done
    
    echo ""
    print_color $GREEN "Total log files: ${#logs[@]}"
    print_color $BLUE "Logs directory: $LOGS_DIR"
}

# Function to get the latest log file
get_latest_log() {
    if [ ! -d "$LOGS_DIR" ]; then
        return 1
    fi
    
    find "$LOGS_DIR" -name "*.log" -type f -exec stat -c "%Y %n" {} \; 2>/dev/null | \
    sort -nr | head -1 | cut -d' ' -f2- || \
    find "$LOGS_DIR" -name "*.log" -type f -exec stat -f "%m %N" {} \; 2>/dev/null | \
    sort -nr | head -1 | cut -d' ' -f2- || return 1
}

# Function to view a log file
view_log() {
    local filename="$1"
    
    if [ "$filename" = "latest" ]; then
        filename=$(get_latest_log)
        if [ -z "$filename" ]; then
            print_color $RED "No log files found"
            return 1
        fi
        print_color $BLUE "Viewing latest log: $(basename "$filename")"
    elif [ ! -f "$LOGS_DIR/$filename" ] && [ ! -f "$filename" ]; then
        print_color $RED "Log file not found: $filename"
        return 1
    elif [ ! -f "$filename" ]; then
        filename="$LOGS_DIR/$filename"
    fi
    
    print_color $BLUE "=== Viewing: $(basename "$filename") ==="
    echo ""
    
    # Use less with color support if available
    if command -v less &> /dev/null; then
        less -R "$filename"
    else
        cat "$filename"
    fi
}

# Function to tail a log file
tail_log() {
    local filename="$1"
    
    if [ "$filename" = "latest" ]; then
        filename=$(get_latest_log)
        if [ -z "$filename" ]; then
            print_color $RED "No log files found"
            return 1
        fi
        print_color $BLUE "Tailing latest log: $(basename "$filename")"
    elif [ ! -f "$LOGS_DIR/$filename" ] && [ ! -f "$filename" ]; then
        print_color $RED "Log file not found: $filename"
        return 1
    elif [ ! -f "$filename" ]; then
        filename="$LOGS_DIR/$filename"
    fi
    
    print_color $BLUE "=== Following: $(basename "$filename") ==="
    print_color $YELLOW "Press Ctrl+C to stop"
    echo ""
    
    tail -f "$filename"
}

# Function to search in log files
search_logs() {
    local pattern="$1"
    
    if [ -z "$pattern" ]; then
        print_color $RED "Search pattern required"
        return 1
    fi
    
    if [ ! -d "$LOGS_DIR" ]; then
        print_color $YELLOW "No logs directory found"
        return 1
    fi
    
    print_color $BLUE "=== Searching for: '$pattern' ==="
    echo ""
    
    local found=false
    for log in $(find "$LOGS_DIR" -name "*.log" -type f | sort -r); do
        if grep -l "$pattern" "$log" &>/dev/null; then
            print_color $GREEN "Found in: $(basename "$log")"
            grep -n --color=always "$pattern" "$log" | head -10
            echo ""
            found=true
        fi
    done
    
    if [ "$found" = false ]; then
        print_color $YELLOW "Pattern '$pattern' not found in any log files"
    fi
}

# Function to clean old logs
clean_logs() {
    local days=${1:-7}
    
    if [ ! -d "$LOGS_DIR" ]; then
        print_color $YELLOW "No logs directory found"
        return 1
    fi
    
    print_color $BLUE "=== Cleaning logs older than $days days ==="
    
    local count=0
    while IFS= read -r -d '' file; do
        print_color $YELLOW "Removing: $(basename "$file")"
        rm "$file"
        ((count++))
    done < <(find "$LOGS_DIR" -name "*.log" -type f -mtime +$days -print0)
    
    if [ $count -eq 0 ]; then
        print_color $GREEN "No logs older than $days days found"
    else
        print_color $GREEN "Removed $count log files"
    fi
}

# Function to show log statistics
show_stats() {
    if [ ! -d "$LOGS_DIR" ]; then
        print_color $YELLOW "No logs directory found"
        return 1
    fi
    
    print_color $BLUE "=== Log Statistics ==="
    echo ""
    
    local total_logs=$(find "$LOGS_DIR" -name "*.log" -type f | wc -l)
    local total_size=$(du -sh "$LOGS_DIR" 2>/dev/null | cut -f1 || echo "Unknown")
    
    print_color $GREEN "Total log files: $total_logs"
    print_color $GREEN "Total disk usage: $total_size"
    
    if [ $total_logs -gt 0 ]; then
        echo ""
        print_color $BLUE "Logs by example type:"
        find "$LOGS_DIR" -name "*.log" -type f -exec basename {} \; | \
        cut -d'_' -f1 | sort | uniq -c | sort -nr | \
        while read count type; do
            echo "  $type: $count files"
        done
        
        echo ""
        print_color $BLUE "Recent logs (last 5):"
        find "$LOGS_DIR" -name "*.log" -type f -exec stat -c "%Y %n" {} \; 2>/dev/null | \
        sort -nr | head -5 | \
        while read timestamp filepath; do
            local date_str=$(date -d "@$timestamp" "+%Y-%m-%d %H:%M" 2>/dev/null || date -r "$timestamp" "+%Y-%m-%d %H:%M" 2>/dev/null || echo "Unknown")
            echo "  $(basename "$filepath") - $date_str"
        done 2>/dev/null || \
        find "$LOGS_DIR" -name "*.log" -type f -exec stat -f "%m %N" {} \; 2>/dev/null | \
        sort -nr | head -5 | \
        while read timestamp filepath; do
            local date_str=$(date -r "$timestamp" "+%Y-%m-%d %H:%M" 2>/dev/null || echo "Unknown")
            echo "  $(basename "$filepath") - $date_str"
        done 2>/dev/null || echo "  Unable to determine recent logs"
    fi
}

# Main script logic
case "${1:-help}" in
    list)
        list_logs
        ;;
    latest)
        latest_log=$(get_latest_log)
        if [ -n "$latest_log" ]; then
            view_log "$latest_log"
        else
            print_color $RED "No log files found"
            exit 1
        fi
        ;;
    view)
        if [ -z "$2" ]; then
            print_color $RED "Filename required for view command"
            show_usage
            exit 1
        fi
        view_log "$2"
        ;;
    tail)
        if [ -z "$2" ]; then
            print_color $RED "Filename required for tail command"
            show_usage
            exit 1
        fi
        tail_log "$2"
        ;;
    search)
        if [ -z "$2" ]; then
            print_color $RED "Search pattern required"
            show_usage
            exit 1
        fi
        search_logs "$2"
        ;;
    clean)
        clean_logs "$2"
        ;;
    stats)
        show_stats
        ;;
    help|--help|-h)
        show_usage
        ;;
    *)
        print_color $RED "Unknown command: $1"
        echo ""
        show_usage
        exit 1
        ;;
esac