import pdfkit

# Replace with the actual path to wkhtmltopdf executable
wkhtmltopdf_path = r"C:\\Program Files\\wkhtmltopdf\\bin\\wkhtmltopdf.exe"

# Set configuration
config = pdfkit.configuration(wkhtmltopdf=wkhtmltopdf_path)

# Convert HTML to PDF with the specified configuration
pdfkit.from_file("input.html", "output.pdf", configuration=config)

print("PDF conversion completed successfully!")
