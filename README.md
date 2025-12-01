
# ⚡AVR Bare Metal SPI Communication Project

_Low-power, bare-metal AVR DD project enabling seamless SPI communication between two microcontrollers with ADC sensing and USART output

---

## 📌 Table of Contents
- <a href="#overview">Overview</a>
- <a href="#Key features">Key features</a>
- <a href="#Hardware requirements">Hardware requirements</a>
- <a href="#Wiring diagram">Wiring diagram</a>
- <a href="#Software Structure">Software Structure</a>
- <a href="#SPI data packet format">SPI data packet format</a>
- <a href="#Power consumption">Power consumption</a>
- <a href="#Building the project">Building the project</a>
- <a href="#Testing">Testing</a>
- <a href="#Troubleshoot">Troubleshoot</a>
- <a href="#File structure">File structure</a>
- <a href="#author--contact">Author & Contact</a>

---
<h2><a class="anchor" id="overview"></a>Overview</h2>

This project implements ultra-low-power communication between two AVR DD microcontrollers using SPI protocol. The HOST device reads an ADC sensor and transmits data to the CLIENT device, which outputs the results via USART.
---
<h2><a class="anchor" id="Key features"></a>Key features</h2>

-Ultra-low power consumption (~1.5µA sleep current)
-State machine-based architecture
-Dynamic clock switching (32.768 kHz ↔ 4 MHz)
-Window comparison ADC
-Interrupt-driven SPI communication

---
<h2><a class="anchor" id="Hardware requirements"></a>Hardware requirements</h2>

Both Devices:

-2x AVR DD Curiosity Nano Development Boards
-USB cables for programming and power
-Logic analyzer (optional, for debugging)

HOST Device Additional:

-Analog sensor (connected to PF2)
-Button switch (built-in on PF6)
---

<h2><a class="anchor" id="Wiring diagram"></a>Wiring diagram</h2>

<span style="color:gray">


│
├── spi_connection/
│   ├── PA4 (MOSI) → PA4 (MOSI)
│   ├── PA5 (MISO) ← PA5 (MISO)
│   ├── PA6 (SCK)  → PA6 (SCK)
│   ├── PA7 (SS)   → PA7 (SS)
│   └── GND        ↔ GND
│
└── host_sensor/
    ├── PC3 → Sensor VCC
    ├── PC2 → Sensor GND
    └── PF2 ← Sensor Analog Out

</span>


---
<h2><a class="anchor" id="project-structure"></a>Project Structure</h2>

```
vendor-performance-analysis/
│
├── README.md
├── .gitignore
├── requirements.txt
├── Vendor Performance Report.pdf
│
├── notebooks/                  # Jupyter notebooks
│   ├── exploratory_data_analysis.ipynb
│   ├── vendor_performance_analysis.ipynb
│
├── scripts/                    # Python scripts for ingestion and processing
│   ├── ingestion_db.py
│   └── get_vendor_summary.py
│
├── dashboard/                  # Power BI dashboard file
│   └── vendor_performance_dashboard.pbix
```

---
<h2><a class="anchor" id="data-cleaning--preparation"></a>Data Cleaning & Preparation</h2>

- Removed transactions with:
  - Gross Profit ≤ 0
  - Profit Margin ≤ 0
  - Sales Quantity = 0
- Created summary tables with vendor-level metrics
- Converted data types, handled outliers, merged lookup tables

---
<h2><a class="anchor" id="exploratory-data-analysis-eda"></a>Exploratory Data Analysis (EDA)</h2>

**Negative or Zero Values Detected:**
- Gross Profit: Min -52,002.78 (loss-making sales)
- Profit Margin: Min -∞ (sales at zero or below cost)
- Unsold Inventory: Indicating slow-moving stock

**Outliers Identified:**
- High Freight Costs (up to 257K)
- Large Purchase/Actual Prices

**Correlation Analysis:**
- Weak between Purchase Price & Profit
- Strong between Purchase Qty & Sales Qty (0.999)
- Negative between Profit Margin & Sales Price (-0.179)

---
<h2><a class="anchor" id="research-questions--key-findings"></a>Research Questions & Key Findings</h2>

1. **Brands for Promotions**: 198 brands with low sales but high profit margins
2. **Top Vendors**: Top 10 vendors = 65.69% of purchases → risk of over-reliance
3. **Bulk Purchasing Impact**: 72% cost savings per unit in large orders
4. **Inventory Turnover**: $2.71M worth of unsold inventory
5. **Vendor Profitability**:
   - High Vendors: Mean Margin = 31.17%
   - Low Vendors: Mean Margin = 41.55%
6. **Hypothesis Testing**: Statistically significant difference in profit margins → distinct vendor strategies

---
<h2><a class="anchor" id="dashboard"></a>Dashboard</h2>

- Power BI Dashboard shows:
  - Vendor-wise Sales and Margins
  - Inventory Turnover
  - Bulk Purchase Savings
  - Performance Heatmaps

![Vendor Performance Dashboard](images/dashboard.png)

---
<h2><a class="anchor" id="how-to-run-this-project"></a>How to Run This Project</h2>

1. Clone the repository:
```bash
git clone https://github.com/yourusername/vendor-performance-analysis.git
```
3. Load the CSVs and ingest into database:
```bash
python scripts/ingestion_db.py
```
4. Create vendor summary table:
```bash
python scripts/get_vendor_summary.py
```
5. Open and run notebooks:
   - `notebooks/exploratory_data_analysis.ipynb`
   - `notebooks/vendor_performance_analysis.ipynb`
6. Open Power BI Dashboard:
   - `dashboard/vendor_performance_dashboard.pbix`

---
<h2><a class="anchor" id="final-recommendations"></a>Final Recommendations</h2>

- Diversify vendor base to reduce risk
- Optimize bulk order strategies
- Reprice slow-moving, high-margin brands
- Clear unsold inventory strategically
- Improve marketing for underperforming vendors

---
<h2><a class="anchor" id="author--contact"></a>Author & Contact</h2>

**Ayushi Mishra**  
Data Analyst  
📧 Email: techclasses0810@gmail.com  
🔗 [LinkedIn](https://www.linkedin.com/in/ayushi-mishra-30813b174/)  
🔗 [Portfolio](https://www.youtube.com/@techclasses0810/)
