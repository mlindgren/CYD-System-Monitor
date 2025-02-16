const maxDataPoints = 30
const cpuData = Array(maxDataPoints).fill(0)
const memoryData = Array(maxDataPoints).fill(0)
const labels = Array(maxDataPoints).fill('')

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
      },
      tooltip: {
        enabled: false
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
      },
      tooltip: {
        enabled: false
      }
    }
  }
})

fetch('/settings')
  .then((response) => response.json())
  .then((data) => {
    const darkMode = data.darkMode
    document.getElementById('darkMode').checked = darkMode
    document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light')
    const textColor = getComputedStyle(document.documentElement).getPropertyValue('--text-color')
    cpuChart.options.scales.y.ticks.color = textColor
    memoryChart.options.scales.y.ticks.color = textColor
    cpuChart.update()
    memoryChart.update()
    document.getElementById('glancesHost').value = data.glances_host
    document.getElementById('glancesPort').value = data.glances_port
    updateServerDisplay()
  })

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

  const quality = Math.max(0, Math.min(100, (dbm + 90) * 2.5))

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
    bar.classList.remove('excellent', 'good', 'fair', 'poor')
    if (isActive) {
      bar.classList.add(getStrengthClass(index, quality))
    }
  })
}

function updateVisualizations(data) {
  const cpuUsage = parseFloat(data.cpuUsage)
  cpuData.push(cpuUsage)
  cpuData.shift()
  cpuChart.update()
  document.querySelector(
    '.chart-card:nth-child(1) h2'
  ).innerHTML = `<i class="ri-cpu-line"></i> CPU Usage <span class="current-value">${cpuUsage.toFixed(1)}%</span>`
  const totalHeap = parseFloat(data.totalHeap)
  const freeHeap = parseFloat(data.freeHeap)
  const usedHeap = totalHeap - freeHeap
  const memoryUsage = Math.max(0, Math.min(100, (usedHeap / totalHeap) * 100))
  memoryData.push(memoryUsage)
  memoryData.shift()
  memoryChart.update()
  document.querySelector(
    '.chart-card:nth-child(2) h2'
  ).innerHTML = `<i class="ri-database-2-line"></i> Memory <span class="current-value">${memoryUsage.toFixed(
    1
  )}%</span>`
  const temp = parseFloat(data.temperature)
  tempGauge.set(temp)
  document.querySelector(
    '.chart-card:nth-child(3) h2'
  ).innerHTML = `<i class="ri-temp-hot-line"></i> Temperature <span class="current-value">${temp.toFixed(1)}°C</span>`
  document.querySelector(
    '.chart-card:nth-child(4) h2'
  ).innerHTML = `<i class="ri-wifi-line"></i> WiFi Signal <span class="current-value">${data.wifiStrength} dBm</span>`
  updateSignalStrength(data.wifiStrength)
}

function updateTheme() {
  const darkMode = document.getElementById('darkMode').checked
  document.documentElement.setAttribute('data-theme', darkMode ? 'dark' : 'light')

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
  Object.keys(data).forEach((key) => {
    const element = document.getElementById(key)
    if (element) {
      let value = data[key]

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

let dataReceived = false
const loader = document.querySelector('.loader-container')

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
