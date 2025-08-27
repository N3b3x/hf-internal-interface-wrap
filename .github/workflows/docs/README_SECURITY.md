# Security Policy

## 🛡️ Reporting Security Vulnerabilities

We take the security of the HardFOC Internal Interface Wrapper project seriously. If you believe you have found a security vulnerability, please report it to us responsibly.

### 🔒 Private Disclosure (Preferred)

For security vulnerabilities, please **do not** open a public GitHub issue. Instead:

1. **Email**: nebysma@gmail.com
2. **Subject**: `[SECURITY] HardFOC Interface Wrapper - [Brief Description]`
3. **Include**:
   - Detailed description of the vulnerability
   - Steps to reproduce
   - Potential impact assessment
   - Suggested mitigation (if any)

### 📋 Response Timeline

- **Initial Response**: Within 48 hours
- **Assessment**: Within 1 week  
- **Fix Development**: 2-4 weeks (depending on severity)
- **Public Disclosure**: After fix is released

## 📦 Supported Versions

We actively maintain security updates for:

| Version | Supported          | ESP-IDF Version |
| ------- | ------------------ | --------------- |
| 1.0.x   | ✅ Active support  | 5.5+           |
| 0.9.x   | ⚠️ Limited support | 5.0+           |
| < 0.9   | ❌ Not supported   | < 5.0          |

## 🔐 Security Features

Our project implements multiple security layers:

- **Automated Dependency Scanning**: Weekly vulnerability checks with Dependabot
- **Secrets Management**: Secure handling of API keys and credentials  
- **Code Security Analysis**: Static analysis and vulnerability scanning
- **ESP32 Hardware Security**: Secure boot, flash encryption, and hardware RNG
- **CI/CD Security**: Secure build pipelines and secret protection

## 📚 Comprehensive Security Documentation

For detailed security guidelines, best practices, and implementation details, see our comprehensive security documentation:

**[📖 Complete Security Policy & Guidelines](docs/security/README_SECURITY.md)**

This includes:
- Development security guidelines
- ESP32-specific security recommendations
- Dependency management policies
- Incident response procedures
- Security monitoring and metrics

## 🚨 Security Incident Classification

- **Critical**: Remote code execution, privilege escalation
- **High**: Data exposure, authentication bypass
- **Medium**: Information disclosure, DoS conditions
- **Low**: Minor security improvements

## 🔧 Quick Security Checklist for Contributors

- [ ] No hardcoded credentials or API keys
- [ ] Use GitHub Secrets for sensitive information
- [ ] Validate and sanitize all inputs
- [ ] Follow secure coding practices
- [ ] Review security documentation before contributing

## 📞 Contact

- **Security Issues**: nebysma@gmail.com (Private disclosure)
- **General Questions**: Create GitHub issue with `security` label
- **Emergency**: nebysma@gmail.com (Subject: `[URGENT SECURITY]`)

---

**Thank you for helping keep HardFOC Interface Wrapper secure!**