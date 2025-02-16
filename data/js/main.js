// Add data arrays for the charts
const maxDataPoints = 30
const cpuData = Array(maxDataPoints).fill(0)
const memoryData = Array(maxDataPoints).fill(0)
const labels = Array(maxDataPoints).fill('')

// Initialize line charts
const cpuChart = new Chart(document.getElementById('cpuChart').getContext('2d'), {
  type: 'line',
  data: {
    labels: labels,
    datasets: [
      {
        label: 'CPU Usage %',
        data: cpuData,
        borderColor: getComputedStyle(document.documentElement).getPropertyValue('--primary-color'),
        tension: 0.4,
        fill: false
      }
    ]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      y: {
        beginAtZero: true,
        max: 100,
        ticks: {
          color: getComputedStyle(document.documentElement).getPropertyValue('--text-color')
        },
        grid: {
          color: getComputedStyle(document.documentElement).getPropertyValue('--border-color')
        }
      },
      x: {
        display: false
      }
    },
    plugins: {
      legend: {
        display: false
      }
    }
  }
})

const memoryChart = new Chart(document.getElementById('memoryChart').getContext('2d'), {
  type: 'line',
  data: {
    labels: labels,
    datasets: [
      {
        label: 'Memory Usage %',
        data: memoryData,
        borderColor: getComputedStyle(document.documentElement).getPropertyValue('--success-color'),
        tension: 0.4,
        fill: false
      }
    ]
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      y: {
        beginAtZero: true,
        max: 100,
        ticks: {
          color: getComputedStyle(document.documentElement).getPropertyValue('--text-color')
        },
        grid: {
          color: getComputedStyle(document.documentElement).getPropertyValue('--border-color')
        }
      },
      x: {
        display: false
      }
    },
    plugins: {
      legend: {
        display: false
      }
    }
  }
})

// Initialize theme state from server
fetch('/settings')
  .then((response) => response.json())
  .then((data) => {
    const darkMode = data.darkMode
    document.getElementById('darkMode').checked = darkMode
    document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light')

    // Set initial chart text colors
    const textColor = getComputedStyle(document.documentElement).getPropertyValue('--text-color')
    cpuChart.options.scales.y.ticks.color = textColor
    memoryChart.options.scales.y.ticks.color = textColor
    cpuChart.update()
    memoryChart.update()

    // Initialize Glances settings
    document.getElementById('glancesHost').value = data.glances_host
    document.getElementById('glancesPort').value = data.glances_port
    updateServerDisplay()
  })

// Initialize gauge with Gauge.js
const tempGauge = new Gauge(document.getElementById('tempGauge')).setOptions({
  angle: 0,
  lineWidth: 0.3,
  radiusScale: 0.9,
  pointer: {
    length: 0.5,
    strokeWidth: 0.035,
    color: getComputedStyle(document.documentElement).getPropertyValue('--danger-color')
  },
  limitMax: true,
  limitMin: true,
  colorStart: getComputedStyle(document.documentElement).getPropertyValue('--danger-color'),
  colorStop: getComputedStyle(document.documentElement).getPropertyValue('--danger-color'),
  strokeColor: getComputedStyle(document.documentElement).getPropertyValue('--border-color'),
  generateGradient: true,
  highDpiSupport: true,
  percentColors: [
    [0.0, '#10b981'],
    [0.5, '#eab308'],
    [1.0, '#ef4444']
  ],
  renderTicks: {
    divisions: 5,
    divWidth: 1.1,
    divLength: 0.7,
    divColor: getComputedStyle(document.documentElement).getPropertyValue('--border-color'),
    subDivisions: 3,
    subLength: 0.5,
    subWidth: 0.6,
    subColor: getComputedStyle(document.documentElement).getPropertyValue('--border-color')
  }
})

tempGauge.maxValue = 100
tempGauge.setMinValue(0)
tempGauge.animationSpeed = 32
tempGauge.set(0)

function updateSignalStrength(dbm) {
  const bars = document.querySelectorAll('.signal-bar')
  const value = document.getElementById('signalValue')
  value.textContent = dbm + ' dBm'

  // Convert dBm to quality percentage
  // Typical WiFi range is -50 dBm (excellent) to -90 dBm (poor)
  const quality = Math.max(0, Math.min(100, (dbm + 90) * 2.5))

  // Define signal strength ranges
  const getStrengthClass = (barIndex, quality) => {
    const threshold = (barIndex + 1) * 25
    if (quality >= threshold) {
      if (quality >= 75) return 'excellent'
      if (quality >= 50) return 'good'
      if (quality >= 25) return 'fair'
      return 'poor'
    }
    return ''
  }

  bars.forEach((bar, index) => {
    const threshold = (index + 1) * 25
    const isActive = quality >= threshold
    bar.classList.toggle('active', isActive)

    // Remove all strength classes first
    bar.classList.remove('excellent', 'good', 'fair', 'poor')

    // Add appropriate strength class if active
    if (isActive) {
      bar.classList.add(getStrengthClass(index, quality))
    }
  })
}

function updateVisualizations(data) {
  // Update CPU chart
  cpuData.push(parseFloat(data.cpuUsage))
  cpuData.shift()
  cpuChart.update()

  // Update memory chart
  const totalHeap = parseFloat(data.totalHeap)
  const freeHeap = parseFloat(data.freeHeap)
  const usedHeap = totalHeap - freeHeap
  const memoryUsage = Math.max(0, Math.min(100, (usedHeap / totalHeap) * 100))
  memoryData.push(memoryUsage)
  memoryData.shift()
  memoryChart.update()

  // Update temperature gauge
  const temp = parseFloat(data.temperature)
  tempGauge.set(temp)
  document.getElementById('tempValue').textContent = `${temp.toFixed(1)}°C`

  // Update WiFi signal strength
  updateSignalStrength(data.wifiStrength)
}

function updateTheme() {
  const darkMode = document.getElementById('darkMode').checked
  document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light')

  // Update chart text colors
  const textColor = getComputedStyle(document.documentElement).getPropertyValue('--text-color')
  cpuChart.options.scales.y.ticks.color = textColor
  memoryChart.options.scales.y.ticks.color = textColor
  cpuChart.update()
  memoryChart.update()

  fetch('/settings', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ darkMode })
  })
}

function formatUptime(seconds) {
  const days = Math.floor(seconds / 86400)
  const hours = Math.floor((seconds % 86400) / 3600)
  const minutes = Math.floor((seconds % 3600) / 60)
  const secs = seconds % 60

  if (days > 0) return `${days}d ${hours}h ${minutes}m`
  if (hours > 0) return `${hours}h ${minutes}m ${secs}s`
  if (minutes > 0) return `${minutes}m ${secs}s`
  return `${secs}s`
}

function updateThemeColor(property, value) {
  const darkMode = document.getElementById('darkMode').checked
  const color = parseInt(value.substring(1), 16)

  fetch('/settings', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      darkMode: darkMode,
      [property]: color
    })
  }).then(() => {
    setTimeout(updateColorPickers, 100)
  })
}

function updateColorPickers() {
  fetch('/settings')
    .then((response) => response.json())
    .then((data) => {
      const pickers = {
        bg_color: 'bgColor',
        card_bg_color: 'cardBgColor',
        text_color: 'textColor',
        cpu_color: 'cpuColor',
        ram_color: 'ramColor',
        border_color: 'borderColor'
      }

      Object.entries(pickers).forEach(([key, id]) => {
        const picker = document.getElementById(id)
        if (picker) {
          picker.value = data[id]
        }
      })
    })
}

function resetTheme() {
  fetch('/resetTheme', {
    method: 'POST'
  })
    .then((response) => response.json())
    .then((data) => {
      if (data.status === 'success') {
        window.location.reload()
      }
    })
}

function restartESP() {
  if (confirm('Are you sure you want to restart the ESP32?')) {
    fetch('/restart', { method: 'POST' }).then(() => {
      alert('ESP32 is restarting...')
      setTimeout(() => {
        window.location.reload()
      }, 5000)
    })
  }
}

function updateSystemInfo(data) {
  // Update all the new fields
  Object.keys(data).forEach((key) => {
    const element = document.getElementById(key)
    if (element) {
      let value = data[key]

      // Add units where appropriate
      if (key === 'cpuFreqMHz') value += ' MHz'
      if (key === 'flashChipSpeed') value += ' MHz'
      if (key.includes('Size') || key.includes('Heap') || key.includes('Space')) value += ' KB'
      if (key === 'temperature') value += ' °C'
      if (key === 'wifiStrength') value += ' dBm'
      if (key === 'heapFragmentation') value += '%'

      element.textContent = value
    }
  })
}

let isEditing = false

function toggleServerEdit() {
  const displayEl = document.querySelector('.server-display')
  const editEl = document.querySelector('.server-edit')
  editEl.classList.toggle('active')
  displayEl.style.display = editEl.classList.contains('active') ? 'none' : 'flex'
  isEditing = editEl.classList.contains('active')
}

function updateServerDisplay() {
  const host = document.getElementById('glancesHost').value
  const port = document.getElementById('glancesPort').value
  const displayEl = document.getElementById('serverDisplay')

  if (host && port) {
    displayEl.textContent = `${host}:${port}`
  } else {
    displayEl.textContent = 'Not configured'
  }
}

// Add at the start of the file
let dataReceived = false
const loader = document.querySelector('.loader-container')

// Modify the interval fetch
setInterval(() => {
  fetch('/settings')
    .then((response) => response.json())
    .then((data) => {
      if (!dataReceived) {
        dataReceived = true
        loader.classList.add('hidden')
      }
      updateSystemInfo(data)
      updateVisualizations(data)
      updateColorPickers()

      if (!isEditing) {
        document.getElementById('glancesHost').value = data.glances_host
        document.getElementById('glancesPort').value = data.glances_port
        updateServerDisplay()
      }
    })
    .catch((error) => {
      console.error('Error:', error)
      dataReceived = false
      loader.classList.remove('hidden')
    })
}, 2000)

// Modify the saveGlancesSettings function
function saveGlancesSettings() {
  const host = document.getElementById('glancesHost').value
  const port = parseInt(document.getElementById('glancesPort').value)

  if (!host || !port) {
    alert('Please enter both host and port')
    return
  }

  fetch('/settings', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({
      glances_host: host,
      glances_port: port
    })
  })
    .then((response) => response.json())
    .then((data) => {
      if (data.status === 'success') {
        updateServerDisplay()
        toggleServerEdit()
        alert('Glances settings updated successfully!')
      }
    })
    .catch((error) => {
      console.error('Error:', error)
      alert('Failed to update Glances settings')
    })
}
