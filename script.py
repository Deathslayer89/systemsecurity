import os

def find_md_files(folder):
    """
    Recursively find all .md files in the given folder.
    """
    md_files = []
    for root, _, files in os.walk(folder):
        for file in files:
            if file.endswith(".md"):
                md_files.append(os.path.join(root, file))
    return md_files

def combine_md_files(md_files, output_md_path):
    """
    Combine the content of all .md files into a single .md file.
    """
    combined_content = ""
    for md_file in md_files:
        with open(md_file, "r", encoding="utf-8") as f:
            combined_content += f.read() + "\n\n"  # Add spacing between files
    with open(output_md_path, "w", encoding="utf-8") as f:
        f.write(combined_content)

def main():
    """
    Main function to combine .md files.
    """
    # Get the current working directory
    folder_path = os.getcwd()

    # Find all .md files in the folder
    md_files = find_md_files(folder_path)
    if not md_files:
        return  # Exit if no .md files are found

    # Define output path for the combined .md file
    output_md_path = os.path.join(folder_path, "combined_output.md")

    # Combine .md files
    combine_md_files(md_files, output_md_path)

if __name__ == "__main__":
    main()