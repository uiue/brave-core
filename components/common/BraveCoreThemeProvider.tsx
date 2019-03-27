import * as React from 'react'
import { ThemeProvider } from 'brave-ui/theme'
import IBraveTheme from 'brave-ui/theme/theme-interface'

export type BraveCoreThemeProviderProps = {
  initialThemeType?: chrome.braveTheme.ThemeType
  dark: IBraveTheme,
  light: IBraveTheme
}
type BraveCoreThemeProviderState = {
  braveCoreTheme?: chrome.braveTheme.ThemeType
}

export default class BraveCoreThemeProvider

extends React.Component<BraveCoreThemeProviderProps, BraveCoreThemeProviderState> {
  componentWillMount () {
    if (this.props.initialThemeType) {
      this.setThemeState(this.props.initialThemeType)
    }
    chrome.braveTheme.onBraveThemeTypeChanged.addListener(this.setThemeState)
  }

  setThemeState = (braveCoreTheme: chrome.braveTheme.ThemeType) => {
    this.setState({ braveCoreTheme })
  }

  render () {
    // Don't render until we have a theme
    if (!this.state.braveCoreTheme) return null
    // Render provided dark or light theme
    const selectedShieldsTheme = this.state.braveCoreTheme === 'Dark'
                                  ? this.props.dark
                                  : this.props.light
    return (
      <ThemeProvider theme={selectedShieldsTheme}>
        {React.Children.only(this.props.children)}
      </ThemeProvider>
    )
  }
}
